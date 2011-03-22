#include "iofwd/rpcfrontend/IOFSLRPCWriteRequest.hh"
#include "iofwdutil/tools.hh"
#include "encoder/EncoderString.hh"
#include "IOFSLRPCGenProcess.hh"
namespace iofwd
{
   namespace rpcfrontend
   {

      void IOFSLRPCWriteRequest::decode()
      {             
         process (dec_, dec_struct.handle);
         process (dec_, dec_struct.mem_count);
         process (dec_, encoder::EncVarArray(dec_struct.mem_sizes, 
                                             dec_struct.mem_count));
         process (dec_, dec_struct.file_count);
         process (dec_, encoder::EncVarArray(&dec_struct.file_starts, 
                                             dec_struct.file_count));
         process (dec_, encoder::EncVarArray(&dec_struct.file_sizes, 
                                             dec_struct.file_count));
         process (dec_, dec_struct.pipeline_size);
         zoidfs::hints::zoidfs_hint_create(&op_hint_);  
         decodeOpHint(&op_hint_); 
      }

      void IOFSLRPCWriteRequest::encode()
      {
          int returnCode = getReturnCode();
          // @TODO: Where are the file_sizes/file_starts values suppose to be
          //        obtained from? 
          enc_struct.file_sizes = dec_struct.file_sizes;
          enc_struct.file_starts = dec_struct.file_starts;
          process (enc_, returnCode);
          process (enc_, encoder::EncVarArray(&enc_struct.file_sizes, dec_struct.file_count));
      }

      IOFSLRPCWriteRequest::ReqParam & IOFSLRPCWriteRequest::decodeParam() 
      { 
          decodeRPCInput(); 
          param_.handle = &dec_struct.handle;
          param_.mem_starts = (char **)dec_struct.mem_starts;
          param_.mem_sizes = dec_struct.mem_sizes;
          param_.mem_count = dec_struct.mem_count;
          param_.file_count = dec_struct.file_count;
          param_.file_sizes = &dec_struct.file_sizes;
          param_.file_starts = &dec_struct.file_starts;
          param_.pipeline_size = dec_struct.pipeline_size;
          param_.op_hint = &op_hint_; 
          return param_; 
      }
      void IOFSLRPCWriteRequest::initRequestParams(ReqParam & p, void * bufferMem)
      {
          // allocate buffer for normal mode
          if (param_.pipeline_size == 0)
          {
              char * mem = NULL;
              for(size_t i = 0 ; i < param_.mem_count ; i++)
              {
                  param_.mem_total_size += p.mem_sizes[i];
              }

              // setup the BMI buffer to the user requested size
              mem = static_cast<char *>(bufferMem);

              // NOTICE: mem_starts_ and mem_sizes_ are alignend with file_sizes
              // This is for request scheduler to easily handle the ranges without
              // extra memory copying.

              // only going to reallocate mem_sizes_ if the mem and file counts are different
              if(param_.mem_count != param_.file_count)
              {
                  param_.mem_count = param_.file_count;
                  delete[] param_.mem_sizes;
                  param_.mem_sizes = new size_t[param_.file_count];
              }

              param_.mem_starts = new char*[param_.file_count];

              // setup the mem offset buffer
              size_t cur = 0;
              for(size_t i = 0; i < param_.file_count ; i++)
              {
                  param_.mem_starts[i] = mem + cur;
                  param_.mem_sizes[i] = param_.file_sizes[i];
                  cur += param_.file_sizes[i];
              }
              p = param_;
          }
      }

      void IOFSLRPCWriteRequest::allocateBuffer(iofwdevent::CBType cb, RetrievedBuffer * rb)
      {
          /* allocate the buffer wrapper */
          rb->buffer_ = new iofwdutil::mm::BMIMemoryAlloc(*addr_, iofwdutil::bmi::BMI::ALLOC_RECEIVE, rb->getsize());

          iofwdutil::mm::BMIMemoryManager::instance().alloc(cb, rb->buffer_);
      }

      void IOFSLRPCWriteRequest::releaseBuffer(RetrievedBuffer * rb)
      {
          iofwdutil::mm::BMIMemoryManager::instance().dealloc(rb->buffer_);

          delete rb->buffer_;
          rb->buffer_ = NULL;
      }

      size_t IOFSLRPCWriteRequest::readBuffer (void * buff, size_t size, bool forceSize)
      {
          size_t readSize = 0;
          size_t outSize = 0;
          void ** tmpBuffer = (void **)new char *[0];
          CBType nullcb = boost::bind(&iofwd::rpcfrontend::IOFSLRPCWriteRequest::NullCB, this,_1);                            
          in_->read((const void **)&tmpBuffer[0], &outSize, nullcb, size);
          /* If the return size MUST match the read size, Recursively call 
             the read with the memory pointer moved (this needs to be checked
             for correctness) */
          // @TODO: Check to make sure this is valid
          memcpy (buff, tmpBuffer, outSize);
          if (forceSize == TRUE)
            readSize = readBuffer (&((char *)buff)[outSize], size - outSize, 
                                   forceSize);
          
          return readSize + outSize;
      }

      void IOFSLRPCWriteRequest::recvBuffers(const CBType & cb, RetrievedBuffer * rb)
      {
          int i = 0;
          CBType nullcb = boost::bind(&iofwd::rpcfrontend::IOFSLRPCWriteRequest::NullCB, this,_1);  
          for (i = 0; i < param_.mem_count; i++)
          {
            readBuffer(param_.mem_starts[i], param_.mem_sizes[i], TRUE);
          }
          
          cb(*(new iofwdevent::CBException()));
      }

      void IOFSLRPCWriteRequest::recvPipelineBufferCB(iofwdevent::CBType cb, RetrievedBuffer * rb, size_t size)
      {
         readBuffer ((rb->buffer_)->getMemory(), size, TRUE);
          cb(*(new iofwdevent::CBException()));
      }

      void IOFSLRPCWriteRequest::reply(const CBType & UNUSED(cb))
      {
          /* verify the args are OK */
          ASSERT(getReturnCode() != zoidfs::ZFS_OK);

          /* encode */
          encodeRPCOutput();

          /* invoke the callback */
          //cb();
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
