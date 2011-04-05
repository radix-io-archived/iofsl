#include "iofwd/rpcfrontend/IOFSLRPCReadRequest.hh"
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

      void IOFSLRPCReadRequest::decode()
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
      }

      void IOFSLRPCReadRequest::encode()
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

      IOFSLRPCReadRequest::ReqParam & IOFSLRPCReadRequest::decodeParam() 
      { 
          decode(); 
          param_.handle = &dec_struct.handle_;
          param_.mem_starts = dec_struct.mem_starts_;
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

      void IOFSLRPCReadRequest::reply(const CBType & UNUSED(cb))
      {
          /* verify the args are OK */
          ASSERT(getReturnCode() == zoidfs::ZFS_OK);

          /* encode */
          encode();

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

      size_t IOFSLRPCReadRequest::writeBuffer(void * buff, size_t size, bool flush)
      {
          size_t outsize = 0;
          size_t ret = 0;
          iofwdevent::SingleCompletion block;
          char ** writePtr = new char *[param_.mem_count];
          block.reset();
          out_->write((void **)writePtr, &outsize, block, size );
          block.wait();
          if (outsize > size)
          {
            memcpy (*writePtr, (char *)buff, size);
            ret = size;
          }
          else
          {
            memcpy (*writePtr, (char *)buff, outsize);
            ret = outsize;
          }
         block.reset();
         out_->flush(block);
         block.wait();   
         delete[] (char **)writePtr;
         return ret;
      }

      void IOFSLRPCReadRequest::allocateBuffer(iofwdevent::CBType cb, RetrievedBuffer * rb)
      {
          /* allocate the buffer wrapper */
          rb->buffer_ = new iofwdutil::mm::GenericMemoryManager();
          rb->buffer_->alloc (rb->getsize());
          cb( *(new iofwdevent::CBException()));
      }

      void IOFSLRPCReadRequest::releaseBuffer(RetrievedBuffer * rb)
      {
          delete rb->buffer_;
          rb->buffer_ = NULL;
      }

      void IOFSLRPCReadRequest::sendBuffers(const iofwdevent::CBType & cb, RetrievedBuffer * rb)
      {
         new boost::thread(boost::bind(&IOFSLRPCReadRequest::sendBuffersBlock, this, cb, rb));  
      }
      void IOFSLRPCReadRequest::sendBuffersBlock(const iofwdevent::CBType & cb, RetrievedBuffer * rb)
      {
          int i = 0;
          size_t outSize = 0;
          size_t readSize = 0;  
          do
          {
            // @TODO Possible read bug here
            outSize += writeBuffer((void*)&(((char**)param_.mem_starts)[i][outSize]), param_.mem_sizes[i] - outSize, TRUE);
            if (outSize == param_.mem_sizes[i])
            {
               i++;
               outSize = 0;
            }
          } while ( i < param_.mem_count);
          cb(*(new iofwdevent::CBException()));
      }

      void IOFSLRPCReadRequest::sendPipelineBufferCB (const iofwdevent::CBType cb, 
                                                      RetrievedBuffer * rb, 
                                                      size_t size) 
      {
         ASSERT("THIS SHOULD NOT BE USED" == 0);
      }
   }
}
