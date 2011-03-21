#include "iofwd/rpcfrontend/IOFSLRPCReadRequest.hh"
#include "iofwdutil/tools.hh"
#include "encoder/EncoderString.hh"
#include "IOFSLRPCGenProcess.hh"
namespace iofwd
{
   namespace rpcfrontend
   {

      zoidfs::zoidfs_handle_t * handle;
      size_t mem_count;
      void ** mem_starts;
      size_t * mem_sizes;
      size_t file_count;
      zoidfs::zoidfs_file_ofs_t * file_starts;
      zoidfs::zoidfs_file_ofs_t * file_sizes;
      size_t pipeline_size;
      zoidfs::zoidfs_op_hint_t * op_hint;

      void IOFSLRPCReadRequest::decode()
      {             
          process(dec_, &dec_struct.handle);
          process(dec_, dec_struct.mem_count);
          process(dec_, encoder::EncVarArray(dec_struct.mem_sizes, dec_struct.mem_count));
          process(dec_, dec_struct.file_count);
          process(dec_, encoder::EncVarArray(dec_struct.file_starts, dec_struct.file_count));
          process(dec_, encoder::EncVarArray(dec_struct.file_sizes, dec_struct.file_count));
          process(dec_, dec_struct.pipeline_size);
          zoidfs::hints::zoidfs_hint_create(&op_hint_);  
          decodeOpHint(&op_hint_); 
      }

      void IOFSLRPCReadRequest::encode()
      {
          int returnCode = getReturnCode();
          enc_struct.file_sizes = file_sizes;
          enc_struct.file_starts = file_starts;
          process (enc_, returnCode);
          process (enc_, encoder::EncVarArray(enc_struct.file_sizes, enc_struct.file_count));
      }

      const IOFSLRPCReadRequest::ReqParam & IOFSLRPCCreateRequest::decodeParam() 
      { 
          decodeRPCInput(); 
          param_.handle = &dec_struct.handle;
          param_.mem_starts = dec_struct.mem_starts;
          param_.mem_sizes = dec_struct.mem_sizes;
          param_.mem_count = dec_struct.mem_count;
          param_.file_count = dec_struct.file_count;
          param_.file_sizes = dec_struct.file_sizes;
          param_.file_starts = dec_struct.file_starts;
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
        void ** writePtr = (void *)new char[param_.mem_count];
        int i = 0;
        int writeSize = size;
        size_t in_offset = 0;
        size_t buff_offset  = 0;
        CBType nullcb = boost::bind(&NullCB, _1);  
        while (in_offset != size)
        {
            out_.write(&writePtr[i], &outsize, nullcb, size - in_offset);
            assert (outsize != 0);

            /* for non-pipeline case */
            if (flush == FALSE)
              assert (size <= outsize);  
            
            if (outsize > size)
            {
              memcpy (&(writePtr[i][in_offset]), &buff[buff_offset], size);
              in_offset = size;
              out_.rewindOutput(outsize - size, nullcb);              
            }
            else
            {
              memcpy (&(writePtr[i][in_offset]), &buff[buff_offset], outsize);
              in_offset = in_offset + outsize;
              buff_offset = buff_offset + outsize;
              out_.flush(nullcb);
              i++;
            }
        }
        delete[] (char **)writePtr;
      }

      void IOFSLRPCReadRequest::sendBuffers(const iofwdevent::CBType & cb, RetrievedBuffer * rb)
      {
          int i = 0;
          CBType nullcb = boost::bind(&NullCB, _1);
          for (i = 0; i < param_.mem_count; i++)
          {
               writeBuffer (rb->buffer_->getMemory()[param_.mem_starts[i]], 
                            param_.mem_sizes[i], FALSE);
          }
          out_.flush(nullcb)
          cb();
      }
      void IOFSLRPCReadRequest::sendPipelineBufferCB (const iofwdevent::CBType cb, 
                                                      RetrievedBuffer * rb, 
                                                      size_t size) 
      {
         writeBuffer (rb_->buffer_->getMemory(), size, TRUE);
         out_.flush(nullcb);
         cb();
      }
   }
}
