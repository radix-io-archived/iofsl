#include "iofwd/rpcfrontend/IOFSLRPCWriteRequest.hh"
#include "iofwdutil/tools.hh"
#include "encoder/EncoderString.hh"
#include "iofwdutil/mm/IOFWDMemoryManager.hh"
#include "IOFSLRPCGenProcess.hh"
#include "iofwdevent/CBException.hh"
#include <memory.h>
#include <boost/thread.hpp>  
namespace iofwd
{
   namespace rpcfrontend
   {

      void IOFSLRPCWriteRequest::decode(const CBType & cb)
      {
         CBType decodeComplete = boost::bind(&IOFSLRPCWriteRequest::processDecode, this, cb);
         insize_ = 15000;

         /* Read Stream */
         in_->read(reinterpret_cast<const void **>(&read_ptr_),
                 &read_size_, decodeComplete, insize_);
      }

      void IOFSLRPCWriteRequest::processDecode(const CBType & cb)
      {
         dec_ = rpc::RPCDecoder(read_ptr_, read_size_);
         process(dec_, dec_struct);
         in_->rewindInput (read_size_ - dec_.getPos(), cb);
      }

      void IOFSLRPCWriteRequest::encode()
      {
         ASSERT("THIS FUNCTION IS TO BE REMOVED!" == 0);        
      }

      void IOFSLRPCWriteRequest::reply(const CBType & cb)
      {
         /* verify the args are OK */
          ASSERT(getReturnCode() == zoidfs::ZFS_OK);
          zoidfs::hints::zoidfs_hint_create(&op_hint_);  
          /* @TODO: Remove this later */
          param_.op_hint = &op_hint_;
          encode(cb);
      }

      void IOFSLRPCWriteRequest::encode(const CBType & cb)
      {
          /* next state to go to to */
          CBType next = boost::bind(&IOFSLRPCWriteRequest::encodeWrite, this, _1, cb);

          outsize_ = 15000;  /* (e->size()).getMaxSize(); */
          
          /* Get Write Location */
          out_->write(reinterpret_cast<void**>(&write_ptr_),
                  &write_size_, next, outsize_);
      }

      void IOFSLRPCWriteRequest::encodeWrite(iofwdevent::CBException e, const CBType & cb)
      {
          e.check();
          /* next state to go to to */
          CBType next = boost::bind(&IOFSLRPCWriteRequest::encodeFlush, this, _1, cb);
          /* Build encoder struct */
          enc_ = rpc::RPCEncoder(write_ptr_, write_size_);

           /* Only returning the return code for now */
          enc_struct.returnCode = getReturnCode();
          process (enc_, enc_struct);

          /* rewind */
          out_->rewindOutput(write_size_ - enc_.getPos(), next);
      }

      void IOFSLRPCWriteRequest::encodeFlush(iofwdevent::CBException e, const CBType & cb)
      {
          e.check();
          if (out_->type == 'T')
             out_->close(cb);   
          else
             out_->flush(cb);
      }
      
      IOFSLRPCWriteRequest::ReqParam & IOFSLRPCWriteRequest::decodeParam() 
      { 
          param_.pipeline_size = 4194304;
          param_.max_buffer_size = 4194304;
          param_.op_hint_pipeline_enabled = true;

          param_.op_hint = &op_hint_;

          /* Values should already be decoded by now */
          param_.handle = &dec_struct.handle;

          param_.mem_count = dec_struct.file.file_count_;
          param_.file_count = dec_struct.file.file_count_;
          param_.file_sizes.reset(dec_struct.file.file_sizes_);
          param_.file_starts.reset(dec_struct.file.file_starts_);

          param_.mem_starts.reset(new char *[param_.file_count]);
          param_.mem_sizes.reset(new size_t[param_.file_count]);
          for (size_t x = 0; x < param_.file_count; x++)
          {
            param_.mem_starts[x] = NULL;
            param_.mem_sizes[x] = param_.file_sizes[x];
          }
          return param_; 
      }
      void IOFSLRPCWriteRequest::initRequestParams(ReqParam & p, void * bufferMem)
      {

          // allocate buffer for normal mode
          if (p.pipeline_size == 0)
          {
              char * mem = NULL;
              for(size_t i = 0 ; i < p.mem_count ; i++)
              {
                  p.mem_total_size += p.mem_sizes[i];
              }

              // setup the BMI buffer to the user requested size
              mem = static_cast<char *>(bufferMem);

              // NOTICE: mem_starts_ and mem_sizes_ are alignend with file_sizes
              // This is for request scheduler to easily handle the ranges without
              // extra memory copying.

              // only going to reallocate mem_sizes_ if the mem and file counts are different
              if(p.mem_count != p.file_count)
              {
                  p.mem_count = p.file_count;
                  p.mem_sizes.reset(new size_t[p.file_count]);
              }

              p.mem_starts.reset(new char*[p.file_count]);

              // setup the mem offset buffer
              size_t cur = 0;
              for(size_t i = 0; i < p.file_count ; i++)
              {
                  p.mem_starts[i] = mem + cur;
                  p.mem_sizes[i] = p.file_sizes[i];
                  cur += p.file_sizes[i];
              }
          }

      }

      void IOFSLRPCWriteRequest::allocateBuffer(iofwdevent::CBType cb, RetrievedBuffer * rb)
      {
          /* allocate the buffer wrapper */
          rb->buffer_ = new iofwdutil::mm::GenericMemoryManager();
          rb->buffer_->alloc (rb->getsize());

          cb( iofwdevent::CBException());
      }

      void IOFSLRPCWriteRequest::releaseBuffer(RetrievedBuffer * rb)
      {
          delete rb->buffer_;
          rb->buffer_ = NULL;
      }

      void IOFSLRPCWriteRequest::recvPipelineBufferCB(iofwdevent::CBType cb, iofwd::RetrievedBuffer* rb, size_t size)
      {
          size_t * outSize = new size_t;
          size_t * runSize = new size_t;
          *outSize = 0;
          *runSize = 0;

          recvRead(cb, rb, size, outSize, runSize);        
      }

      void IOFSLRPCWriteRequest::recvRead (iofwdevent::CBType cb, 
                                           iofwd::RetrievedBuffer* rb, 
                                           size_t size,
                                           size_t * outSize,
                                           size_t * runSize)
      {
        void * loc;
        CBType next = boost::bind(&IOFSLRPCWriteRequest::recvCheck, this, _1, cb, rb,
                                  size, outSize, runSize);
        *runSize += *outSize;
        loc = (void*)&(((char*)rb->buffer_->getMemory())[*runSize]);
        readBuffer( next, (void*)loc, size, outSize);
      }

      void IOFSLRPCWriteRequest::recvCheck (iofwdevent::CBException e,
                                            iofwdevent::CBType cb,
                                            iofwd::RetrievedBuffer* rb, 
                                            size_t size,
                                            size_t * outSize,
                                            size_t * runSize)
      {
        e.check();
        if (*outSize < size)
          recvRead (cb, rb, size - *outSize, outSize, runSize);
        else
        {
          delete outSize;
          delete runSize;
          cb(iofwdevent::CBException());      
        }
      }
  
      void IOFSLRPCWriteRequest::readBuffer (CBType cb, void * buff, size_t sizdec_, size_t * outSize)
      {
         void ** tmpBuffer = (void **)new char * [1];

         CBType next = boost::bind(&IOFSLRPCWriteRequest::bufferRecv, this, _1, cb,
                                   buff, sizdec_, outSize, tmpBuffer);
         *outSize = 0;
         in_->read((const void **)tmpBuffer, outSize, next, sizdec_);
      }


      void IOFSLRPCWriteRequest::bufferRecv (iofwdevent::CBException e, CBType cb,
                                               void * buff, size_t sizdec_,
                                               size_t * outSize, void ** tmpBuffer)     
      {
         e.check();
         if (*outSize < sizdec_)
         {
            std::memcpy ( buff, *tmpBuffer, *outSize);
            delete[] tmpBuffer;
            cb(iofwdevent::CBException());
         }
         else
         {
            std::memcpy ( buff, *tmpBuffer, sizdec_);
            delete[] tmpBuffer;
            in_->rewindInput (*outSize - sizdec_, cb);
         }
      }


      void IOFSLRPCWriteRequest::recvBuffers(const CBType & UNUSED(cb), RetrievedBuffer * UNUSED(rb))
      {
          ASSERT ("Non-Pipeline mode not availible with streams" == 0);
      }


      IOFSLRPCWriteRequest::~IOFSLRPCWriteRequest()
      {
         zoidfs::hints::zoidfs_hint_free(&op_hint_);
      }

      size_t IOFSLRPCWriteRequest::rpcEncodedInputDataSize()
      {
          return 0;
      }

      size_t IOFSLRPCWriteRequest::rpcEncodedOutputDataSize()
      {
          return 0;
      }


   }
}
