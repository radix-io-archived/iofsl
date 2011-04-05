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

      void IOFSLRPCWriteRequest::decode()
      {

         iofwdevent::SingleCompletion block;

         /* sanity */
         block.reset();      

         /* prepare to read from the in stream */
         insize_ = 15000;

         /* Read Stream */
         in_->read(reinterpret_cast<const void **>(&read_ptr_),
                 &read_size_, block, insize_);
         block.wait();
   
         /* Start RPCDecoder */            
         dec_ = rpc::RPCDecoder(read_ptr_, read_size_);

         process (dec_, dec_struct.handle_);
         process (dec_, dec_struct.mem_count_);
         dec_struct.mem_sizes_ = new size_t[dec_struct.mem_count_];
         process (dec_, encoder::EncVarArray(dec_struct.mem_sizes_, dec_struct.mem_count_));

         process (dec_, dec_struct.file_count_);         
         dec_struct.file_starts_  = new zoidfs::zoidfs_file_ofs_t[dec_struct.file_count_];
         dec_struct.file_sizes_   = new zoidfs::zoidfs_file_ofs_t[dec_struct.file_count_];

         process (dec_, encoder::EncVarArray( dec_struct.file_starts_, dec_struct.file_count_));
         process (dec_, encoder::EncVarArray( dec_struct.file_sizes_, dec_struct.file_count_));


         block.reset();
         in_->rewindInput (read_size_ - dec_.getPos(), block);
         block.wait();
  
         /* Hint decoding disabled currently */
//         zoidfs::hints::zoidfs_hint_create(&op_hint_);  
//         decodeOpHint(&op_hint_); 
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
            out_->flush(block);
            block.wait();
      }

      IOFSLRPCWriteRequest::ReqParam & IOFSLRPCWriteRequest::decodeParam() 
      { 
          decode(); 
          param_.handle = &dec_struct.handle_;
          param_.mem_starts = (char **)dec_struct.mem_starts_;
          param_.mem_sizes = dec_struct.mem_sizes_;
          param_.mem_count = dec_struct.mem_count_;
          param_.file_count = dec_struct.file_count_;
          param_.file_sizes = dec_struct.file_sizes_;
          param_.file_starts = dec_struct.file_starts_;

          /* Pipelining no longer matter */
          param_.pipeline_size = 0;

          param_.op_hint = &op_hint_; 
          return param_; 
      }
      void IOFSLRPCWriteRequest::initRequestParams(ReqParam & p, void * bufferMem)
      {
          /* Is this even called anymore? */

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
          rb->buffer_ = new iofwdutil::mm::GenericMemoryManager();
          rb->buffer_->alloc (rb->getsize());
          cb( *(new iofwdevent::CBException()));
      }

      void IOFSLRPCWriteRequest::releaseBuffer(RetrievedBuffer * rb)
      {
          delete rb->buffer_;
          rb->buffer_ = NULL;
      }

      size_t IOFSLRPCWriteRequest::readBuffer (void ** buff, size_t sizdec_, bool forceSize)
      {
         size_t ret = 0;
         size_t outSize = 0;
         void ** tmpBuffer = (void **)new char *[1];
         iofwdevent::SingleCompletion block;
         block.reset();
         in_->read((const void **)&tmpBuffer, &outSize, block, sizdec_);
         block.wait();

//         assert (outSize < sizdec_);

         if (outSize < sizdec_)
         {
            std::memcpy ( *buff, tmpBuffer, outSize);
            ret = outSize;
         }
         else
         {
            std::memcpy ( *buff, tmpBuffer, sizdec_);
            block.reset();
            in_->rewindInput (outSize - sizdec_, block);
            block.wait();
            ret = sizdec_;
         }
        return ret;
      }


      void IOFSLRPCWriteRequest::recvBuffers(const CBType & cb, RetrievedBuffer * rb)
      {
          int i = 0;
          size_t outSize = 0;
          size_t readSize = 0;
//          CBType nullcb = boost::bind(&iofwd::rpcfrontend::IOFSLRPCWriteRequest::NullCB, this,_1);  
          do
          {
            outSize += readBuffer((void**)&param_.mem_starts[i], param_.mem_sizes[i] - outSize, TRUE);
            fprintf(stderr, "OUTSIZE: %i\n", outSize);
            if (outSize == param_.mem_sizes[i])
            {
               i++;
               outSize = 0;
            }
            
          } while ( i < param_.mem_count);
          cb(*(new iofwdevent::CBException()));
      }

      void IOFSLRPCWriteRequest::recvPipelineBufferCB(iofwdevent::CBType cb, RetrievedBuffer * rb, size_t size)
      {
         size_t ret_size = 0;
         size_t total_size = 0;
         int pos = 0;
         void * mem = (rb->buffer_)->getMemory();
         do 
         {
            ret_size = readBuffer (&mem, size, TRUE);
            total_size += ret_size;
            mem = &(((char*)((rb->buffer_)->getMemory()))[total_size]);
         } while (ret_size != 0 || total_size != size);
         cb(*(new iofwdevent::CBException()));
      }

      void IOFSLRPCWriteRequest::reply(const CBType & UNUSED(cb))
      {
          /* verify the args are OK */
//          ASSERT(getReturnCode() != zoidfs::ZFS_OK);
         encode();
          /* encode */
//          encodeRPCOutput();

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
