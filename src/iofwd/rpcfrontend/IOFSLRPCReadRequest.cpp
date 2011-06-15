#include "iofwd/rpcfrontend/IOFSLRPCReadRequest.hh"
#include "iofwdutil/tools.hh"
#include "encoder/EncoderString.hh"
#include "iofwdutil/mm/IOFWDMemoryManager.hh"
#include "IOFSLRPCGenProcess.hh"
#include "iofwdevent/CBException.hh"
#include <memory.h>
#include <boost/thread.hpp>  
#include <cstdio>
namespace iofwd
{
   namespace rpcfrontend
   {

      void IOFSLRPCReadRequest::decode(const CBType & cb)
      {             
         CBType decodeComplete = boost::bind(&IOFSLRPCReadRequest::processDecode, this, cb);         

         insize_ = 15000;

         in_->read(reinterpret_cast<const void **>(&read_ptr_),
                 &read_size_, decodeComplete, insize_); 
      }

      void IOFSLRPCReadRequest::processDecode(const CBType & cb)
      {
         /* Start RPCDecoder */            
         dec_ = rpc::RPCDecoder(read_ptr_, read_size_);
         process(dec_,dec_struct);
         in_->rewindInput (read_size_ - dec_.getPos(), cb);
      }

      void IOFSLRPCReadRequest::encode(CBType cb)
      {
          CBType next = boost::bind(&IOFSLRPCReadRequest::writeEncode, this,
                                    _1, cb);

          outsize_ = 15000;  /* (e->size()).getMaxSize(); */

          /* Get Write Location */
          out_->write(reinterpret_cast<void**>(&write_ptr_),
                  &write_size_, next, outsize_);
      }
      void IOFSLRPCReadRequest::writeEncode (iofwdevent::CBException e, 
                                             CBType cb)
      {
          e.check();
          CBType next = boost::bind(&IOFSLRPCReadRequest::encodeFlush, this,
                                    _1, cb);

          /* Build encoder struct */
          enc_ = rpc::RPCEncoder(write_ptr_, write_size_);

          /* Only returning the return code for now */
          enc_struct.returnCode = getReturnCode();
          process (enc_, enc_struct);

          /* rewind */
          out_->rewindOutput(write_size_ - enc_.getPos(), next);
      }

      void IOFSLRPCReadRequest::encodeFlush (iofwdevent::CBException e,
                                             CBType cb)
      {
          e.check();
          out_->flush(cb);
      }

      IOFSLRPCReadRequest::ReqParam & IOFSLRPCReadRequest::decodeParam() 
      { 
         
          param_.handle = &dec_struct.handle;
          param_.mem_count = dec_struct.file.file_count_;
          param_.file_count = dec_struct.file.file_count_;
          param_.file_sizes.reset(dec_struct.file.file_sizes_);
          param_.file_starts.reset(dec_struct.file.file_starts_);

          param_.mem_starts.reset(new void *[param_.file_count]);
          param_.mem_sizes.reset(new size_t[param_.file_count]);
          for (size_t x = 0; x < param_.file_count; x++)
          {
            param_.mem_starts[x] = NULL;
            param_.mem_sizes[x] = param_.file_sizes[x];
          }

          total_read_size = 0;
          for(size_t i = 0 ; i < param_.mem_count; i++)
          {
               total_read_size += param_.mem_sizes[i];
          }

          param_.pipeline_size = 4194304;
          param_.max_buffer_size = 4194304;
          param_.op_hint_pipeline_enabled = true;
          return param_; 
      }

      void IOFSLRPCReadRequest::reply(const CBType & cb)
      {
         
          /* verify the args are OK */
          ASSERT(getReturnCode() == zoidfs::ZFS_OK);

          /* invoke the callback */
          zoidfs::hints::zoidfs_hint_create(&op_hint_);  
          /* @TODO: Remove this later */
          param_.op_hint = &op_hint_;
         
          cb(iofwdevent::CBException());
      }

      void IOFSLRPCReadRequest::initRequestParams(ReqParam & p, void * bufferMem)
      {
          // allocate buffer for normal mode
          if (p.pipeline_size == 0)
          {
            char * mem = NULL;

            // create the bmi buffer
            mem = static_cast<char *>(bufferMem);

            // NOTICE: mem_starts_ and mem_sizes_ are alignend with file_sizes
            // This is for request scheduler to easily handle the ranges without
            // extra memory copying.

            // only going to reallocate if file and mem counts are diff
            if(p.mem_count != p.file_count)
            {
                p.mem_count = p.file_count;
                p.mem_sizes.reset(new size_t[p.file_count]);
            }

            p.mem_starts.reset(new void*[p.file_count]);

            // setup the mem offset and start buffers
            size_t cur = 0;
            for (size_t i = 0; i < p.file_count; i++)
            {
                p.mem_starts[i] = mem + cur;
                p.mem_sizes[i] = p.file_sizes[i];
                cur += p.file_sizes[i];
            }
         }
         
      }

      void IOFSLRPCReadRequest::allocateBuffer(const iofwdevent::CBType cb, RetrievedBuffer * rb)
      {
          /* allocate the buffer wrapper */
          rb->buffer_ = new iofwdutil::mm::GenericMemoryManager();
          rb->buffer_->alloc (rb->getsize());
          cb(iofwdevent::CBException());
      }


      void IOFSLRPCReadRequest::releaseBuffer(RetrievedBuffer * rb)
      {
          delete rb->buffer_;
          rb->buffer_ = NULL;
      }

      void IOFSLRPCReadRequest::sendBuffers(const iofwdevent::CBType & UNUSED(cb), 
                                            RetrievedBuffer * UNUSED(rb))
      {
         ASSERT ("NON PIPELINE MODE NOT ENABLED" == 0);
      }

      void IOFSLRPCReadRequest::sendPipelineBufferCB (const iofwdevent::CBType cb, 
                                                      RetrievedBuffer * rb, 
                                                      size_t size) 
      {
        size_t * outSize = new size_t;
        size_t * readSize = new size_t;  
        size_t * readLoc = new size_t;
        *outSize = 0;
        *readSize = 0;
        *readLoc = 0;

        CBType next = boost::bind(&IOFSLRPCReadRequest::sendNext, this, _1, cb,
                                  rb, size, outSize, readSize, readLoc);
        /* Write header */
        if (header_sent == false)
        {
          header_sent = true;
          encode(next);
        }
        else
        {
          next(iofwdevent::CBException());
        }
      }

      void IOFSLRPCReadRequest::sendNext ( iofwdevent::CBException e,
                                           const iofwdevent::CBType cb, 
                                           RetrievedBuffer * rb, 
                                           size_t size,
                                           size_t * outSize,
                                           size_t * readSize,
                                           size_t * readLoc) 
      {
          e.check();
          void * loc;
          CBType next = boost::bind(&IOFSLRPCReadRequest::sendCheck, this, _1,
                                    cb, rb, size, outSize, readSize, readLoc);
          loc = &((char*)rb->buffer_->getMemory())[*readLoc];
          writeGetBuffer( next, loc, size - *outSize, readSize);
      }

      void IOFSLRPCReadRequest::sendCheck ( iofwdevent::CBException e,
                                            const iofwdevent::CBType cb, 
                                            RetrievedBuffer * rb, 
                                            size_t size,
                                            size_t * outSize,
                                            size_t * readSize,
                                            size_t * readLoc) 
      {
        e.check();
        *outSize += *readSize;
        *readLoc += *readSize;
        total_read_size -= *readSize;
        if (*outSize != size)
        {
          sendNext(iofwdevent::CBException(), cb, rb, size, outSize, readSize,
                   readLoc);
        }
        else
        {
          delete outSize;
          delete readLoc;
          delete readSize;
          if (total_read_size == 0)
            out_->close(cb);
          else
            cb(iofwdevent::CBException());            
        }
      }

      void IOFSLRPCReadRequest::writeGetBuffer(CBType cb, void * buff, 
                                               size_t size, size_t * readSize)
      {         
          size_t * outsize = new size_t;
          char ** writePtr = new char *[param_.mem_count];
          CBType next = boost::bind (&IOFSLRPCReadRequest::writeGotBuffer, this,
                                     _1, cb, buff, size, readSize, outsize, 
                                     writePtr);
          out_->write((void **)writePtr, outsize, next, size );
      }

      void IOFSLRPCReadRequest::writeGotBuffer(iofwdevent::CBException e, 
                                               CBType cb, void * buff, 
                                               size_t size, size_t * readSize,
                                               size_t * outsize, char ** writePtr)
      {
        e.check();
        if (*outsize > size)
        {
          memcpy (*writePtr, (char *)buff, size);
          *readSize = size;
        }
        else
        {
          memcpy (*writePtr, (char *)buff, *outsize);
          *readSize = *outsize;
        }
        CBType next = boost::bind(&IOFSLRPCReadRequest::writeFlush, this, _1,
                                  cb, outsize, writePtr);
        out_->rewindOutput(*outsize - *readSize, next);
      }

      void IOFSLRPCReadRequest::writeFlush (iofwdevent::CBException e,
                                            CBType cb, size_t * outsize,
                                            char ** writePtr)
      {
        e.check();
        delete outsize;
        delete [] writePtr;
        out_->flush(cb);
      }

      IOFSLRPCReadRequest::~IOFSLRPCReadRequest()
      {
         zoidfs::hints::zoidfs_hint_free(&op_hint_);
      }

      size_t IOFSLRPCReadRequest::rpcEncodedInputDataSize()
      {
          return 0;
      }

      size_t IOFSLRPCReadRequest::rpcEncodedOutputDataSize()
      {
          return 0;
      }

   }
}
