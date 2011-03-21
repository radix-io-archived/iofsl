#include "iofwd/rpcfrontend/IOFSLRPCReadRequest.hh"
#include "iofwdutil/tools.hh"
#include "encoder/EncoderString.hh"
#include "IOFSLRPCGenProcess.hh"
namespace iofwd
{
   namespace rpcfrontend
   {

      void IOFSLRPCReadRequest::decode()
      {             
          process(dec_, dec_struct.handle);
          process(dec_, dec_struct.mem_count);
          process(dec_, encoder::EncVarArray(dec_struct.mem_sizes, dec_struct.mem_count));
          process(dec_, dec_struct.file_count);
          process(dec_, encoder::EncVarArray(&dec_struct.file_starts, dec_struct.file_count));
          process(dec_, encoder::EncVarArray(&dec_struct.file_sizes, dec_struct.file_count));
          process(dec_, dec_struct.pipeline_size);
          zoidfs::hints::zoidfs_hint_create(&op_hint_);  
          decodeOpHint(&op_hint_); 
      }

      void IOFSLRPCReadRequest::encode()
      {
          int returnCode = getReturnCode();
          // @TODO: Where are the file_sizes/file_starts values suppose to be
          //        obtained from? 
          enc_struct.file_sizes = dec_struct.file_sizes;
          enc_struct.file_starts = dec_struct.file_starts;
          process (enc_, returnCode);
          process (enc_, encoder::EncVarArray(&enc_struct.file_sizes, dec_struct.file_count));
      }

      IOFSLRPCReadRequest::ReqParam & IOFSLRPCReadRequest::decodeParam() 
      { 
          decodeRPCInput(); 
          param_.handle = &dec_struct.handle;
          param_.mem_starts = dec_struct.mem_starts;
          param_.mem_sizes = dec_struct.mem_sizes;
          param_.mem_count = dec_struct.mem_count;
          param_.file_count = dec_struct.file_count;
          param_.file_sizes = &dec_struct.file_sizes;
          param_.file_starts = &dec_struct.file_starts;
          param_.pipeline_size = dec_struct.pipeline_size;
          param_.op_hint = &op_hint_; 
          return param_; 
      }

      void IOFSLRPCReadRequest::reply(const CBType & UNUSED(cb))
      {
          /* verify the args are OK */
          ASSERT(getReturnCode() != zoidfs::ZFS_OK);

          /* encode */
          encodeRPCOutput();

          /* invoke the callback */
          //cb();
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

      void IOFSLRPCReadRequest::writeBuffer(void * buff, size_t size, bool flush)
      {
          void ** writePtr = (void **)new char[param_.mem_count];
          int i = 0;
          int writeSize = size;
          size_t outsize = 0;
          size_t in_offset = 0;
          size_t buff_offset  = 0;
          CBType nullcb = boost::bind(&iofwd::rpcfrontend::IOFSLRPCReadRequest::NullCB, this,_1);  
          while (in_offset != size)
          {
              out_->write(&writePtr[i], &outsize, nullcb, size - in_offset);
              assert (outsize != 0);

              /* for non-pipeline case */
              if (flush == FALSE)
                assert (size <= outsize);  
              
              if (outsize > size)
              {
                memcpy (&((char **)writePtr)[i][in_offset], &((char *)buff)[buff_offset], size);
                in_offset = size;
                out_->rewindOutput(outsize - size, nullcb);              
              }
              else
              {
                memcpy (&((char **)writePtr)[i][in_offset], &((char *)buff)[buff_offset], outsize);
                in_offset = in_offset + outsize;
                buff_offset = buff_offset + outsize;
                out_->flush(nullcb);
                i++;
              }
          }
          delete[] (char **)writePtr;
      }

      void IOFSLRPCReadRequest::initRequestParams(ReqParam & p, void * bufferMem)
      {
          // allocate buffer for normal mode
          if (param_.pipeline_size == 0)
          {
            char * mem = NULL;
            // compute the total size of the io op
//            for(size_t i = 0 ; i < param_.mem_count ; i++)
//            {
//                mem_total_size += param_.mem_sizes[i];
//            }

            // create the bmi buffer
            mem = static_cast<char *>(bufferMem);

            // NOTICE: mem_starts_ and mem_sizes_ are alignend with file_sizes
            // This is for request scheduler to easily handle the ranges without
            // extra memory copying.

            // only going to reallocate if file and mem counts are diff
            if(param_.mem_count != param_.file_count)
            {
                param_.mem_count = param_.file_count;
                delete[] param_.mem_sizes;
                param_.mem_sizes = new size_t[param_.file_count];
            }

            param_.mem_starts = new void*[param_.file_count];

            // setup the mem offset and start buffers
            size_t cur = 0;
            for (size_t i = 0; i < param_.file_count; i++)
            {
                param_.mem_starts[i] = mem + cur;
                param_.mem_sizes[i] = param_.file_sizes[i];
                cur += param_.file_sizes[i];
            }
            p = param_;
         }
      }

      void IOFSLRPCReadRequest::allocateBuffer(iofwdevent::CBType cb, RetrievedBuffer * rb)
      {
          /* allocate the buffer wrapper */
          rb->buffer_ = new iofwdutil::mm::BMIMemoryAlloc(*addr_, 
                                              iofwdutil::bmi::BMI::ALLOC_SEND, 
                                              rb->getsize());

          iofwdutil::mm::BMIMemoryManager::instance().alloc(cb, rb->buffer_);
      }

      void IOFSLRPCReadRequest::releaseBuffer(RetrievedBuffer * rb)
      {
          iofwdutil::mm::BMIMemoryManager::instance().dealloc(rb->buffer_);

          delete rb->buffer_;
          rb->buffer_ = NULL;
      }
      void IOFSLRPCReadRequest::sendBuffers(const iofwdevent::CBType & cb, RetrievedBuffer * rb)
      {
          int i = 0;
          CBType nullcb = boost::bind(&iofwd::rpcfrontend::IOFSLRPCReadRequest::NullCB, this,_1);  
          for (i = 0; i < param_.mem_count; i++)
          {
               writeBuffer (param_.mem_starts[i],  param_.mem_sizes[i], FALSE);
          }
          out_->flush(nullcb);
         cb(*(new iofwdevent::CBException()));
      }

      void IOFSLRPCReadRequest::sendPipelineBufferCB (const iofwdevent::CBType cb, 
                                                      RetrievedBuffer * rb, 
                                                      size_t size) 
      {
         CBType nullcb = boost::bind(&iofwd::rpcfrontend::IOFSLRPCReadRequest::NullCB, this,_1);  
         writeBuffer (rb->buffer_->getMemory(), size, TRUE);
         out_->flush(nullcb);
         cb(*(new iofwdevent::CBException()));
      }
   }
}
