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
         /* For transforms, if there is nothing to read (aka a full block has 
            not been decoded) return to decode to read some more */
         if (read_size_ == 0)
            decode(cb);
         /* Start RPCDecoder */            
         dec_ = rpc::RPCDecoder(read_ptr_, read_size_);

         process (dec_, dec_struct.handle_);
         process (dec_, dec_struct.mem_count_);
         dec_struct.mem_starts_ = new void * [dec_struct.mem_count_];
         dec_struct.mem_sizes_ = new size_t[dec_struct.mem_count_];
         process (dec_, encoder::EncVarArray(dec_struct.mem_sizes_, dec_struct.mem_count_));

         process (dec_, dec_struct.file_count_);         
         dec_struct.file_starts_  = new zoidfs::zoidfs_file_ofs_t[dec_struct.file_count_];
         dec_struct.file_sizes_   = new zoidfs::zoidfs_file_ofs_t[dec_struct.file_count_];

         process (dec_, encoder::EncVarArray( dec_struct.file_starts_, dec_struct.file_count_));
         process (dec_, encoder::EncVarArray( dec_struct.file_sizes_, dec_struct.file_count_));

         /* Hint decoding disabled currently */
//         zoidfs::hints::zoidfs_hint_create(&op_hint_);  
//         decodeOpHint(&op_hint_); 

         in_->rewindInput (read_size_ - dec_.getPos(), cb);
      }

      void IOFSLRPCWriteRequest::encodeCB(const CBType & cb)
      {

          boost::this_thread::at_thread_exit(iofwdutil::ThreadPoolKick(*tp_));       
          encode();
          zoidfs::hints::zoidfs_hint_create(&op_hint_);  
          /* @TODO: Remove this later */
          param_.op_hint = &op_hint_;

          cb(iofwdevent::CBException());
      }      

      void IOFSLRPCWriteRequest::encode()
      {

            iofwdevent::SingleCompletion block;
           
            /* sanity */ 
            block.reset();

            // @TODO: Not using actual size until size function is fixed          
            /* Get size of encode */ 
            //process (size_, outStruct);
            outsize_ = 15000;  /* (e->size()).getMaxSize(); */
            
            /* Get Write Location */
            out_->write(reinterpret_cast<void**>(&write_ptr_),
                    &write_size_, block, outsize_);
            block.wait();

            /* Build encoder struct */
            enc_ = rpc::RPCEncoder(write_ptr_, write_size_);
      
             /* Only returning the return code for now */
             int returnCode = getReturnCode();
             process (enc_, returnCode);

            /* sanity */
            block.reset();

            /* rewind */
            out_->rewindOutput(write_size_ - enc_.getPos(), block);
            block.wait();

            /* flush the reponse */
            block.reset();
            if (out_->type == 'T')
               out_->close(block);   
            else
               out_->flush(block);
            block.wait();

      }
      
      IOFSLRPCWriteRequest::ReqParam & IOFSLRPCWriteRequest::decodeParam() 
      { 
          /* Values should already be decoded by now */
//          decode(); 
          param_.handle = &dec_struct.handle_;
          param_.mem_starts.reset((char **)dec_struct.mem_starts_);
          param_.mem_sizes.reset(dec_struct.mem_sizes_);
          param_.mem_count = dec_struct.mem_count_;
          param_.file_count = dec_struct.file_count_;
          param_.file_sizes.reset(dec_struct.file_sizes_);
          param_.file_starts.reset(dec_struct.file_starts_);

          /* Pipelining no longer matter */
          param_.pipeline_size = 4194304;

          param_.max_buffer_size = 4194304;
          param_.op_hint_pipeline_enabled = true;
          param_.op_hint = &op_hint_;
 
          return param_; 
      }
      void IOFSLRPCWriteRequest::initRequestParams(ReqParam & p, void * bufferMem)
      {

          /* Is this even called anymore? */

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

      size_t IOFSLRPCWriteRequest::readBuffer (void ** buff, size_t sizdec_, bool UNUSED(forceSize))
      {

         size_t ret = 0;
         size_t outSize = 0;
         void ** tmpBuffer = (void **)new char * [1];
         iofwdevent::SingleCompletion block;
         block.reset();
         in_->read((const void **)tmpBuffer, &outSize, block, sizdec_);
         block.wait();

         if (outSize < sizdec_)
         {
            std::memcpy ( *buff, *tmpBuffer, outSize);
            ret = outSize;
         }
         else
         {
            std::memcpy ( *buff, *tmpBuffer, sizdec_);
            block.reset();
            in_->rewindInput (outSize - sizdec_, block);
            block.wait();
            ret = sizdec_;
         }
        delete[] tmpBuffer;

        return ret;
      }

      void IOFSLRPCWriteRequest::recvBuffers(const CBType & cb, RetrievedBuffer * rb)
      {

          tp_->submitWorkUnit(boost::bind(&IOFSLRPCWriteRequest::recvBuffersBlock, this, cb, rb),
                               iofwdutil::ThreadPool::HIGH);

      }

      void IOFSLRPCWriteRequest::recvBuffersBlock(const CBType & cb, RetrievedBuffer * rb)
      {

          boost::this_thread::at_thread_exit(iofwdutil::ThreadPoolKick(*tp_)); 
          size_t i = 0;
          size_t outSize = 0;
          void * loc;
          do
          {
            loc = (void*)&(((char*)rb->buffer_->getMemory())[outSize]);
            outSize += readBuffer((void**)&loc, param_.mem_sizes[i] - outSize, TRUE);
            if (outSize == param_.mem_sizes[i])
            {
               i++;
               outSize = 0;
            }
            
          } while ( i < param_.mem_count);

          cb(iofwdevent::CBException());
      }


      void IOFSLRPCWriteRequest::recvPipelineBufferCB(iofwdevent::CBType cb, iofwd::RetrievedBuffer* rb, size_t size)
      {
       
           tp_->submitWorkUnit (boost::bind(&IOFSLRPCWriteRequest::recvPipelineBufferCBBlock, 
                                           this, cb, rb, size), iofwdutil::ThreadPool::HIGH);

      }


      void IOFSLRPCWriteRequest::recvPipelineBufferCBBlock(iofwdevent::CBType cb, iofwd::RetrievedBuffer* rb, size_t size)
      {

          boost::this_thread::at_thread_exit(iofwdutil::ThreadPoolKick(*tp_));           
          size_t outSize = 0;
          
          void * loc;
          do
          {
            loc = (void*)&(((char*)rb->buffer_->getMemory())[outSize]);
            outSize += readBuffer((void**)&loc, size - outSize, TRUE);
          } while (outSize != size);
          cb(iofwdevent::CBException());
      }


      void IOFSLRPCWriteRequest::reply(const CBType & cb)
      {
       
         /* verify the args are OK */
          ASSERT(getReturnCode() == zoidfs::ZFS_OK);
          tp_->submitWorkUnit ( boost::bind (&IOFSLRPCWriteRequest::encodeCB, this, cb),
                                  iofwdutil::ThreadPool::HIGH);

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
