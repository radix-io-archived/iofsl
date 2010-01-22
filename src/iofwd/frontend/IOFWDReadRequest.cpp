#include "IOFWDReadRequest.hh"
#include "iofwdutil/bmi/BMIOp.hh"
#include "zoidfs/zoidfs-proto.h"
#include "iofwd_config.h"

namespace iofwd
{
   namespace frontend
   {
//===========================================================================

IOFWDReadRequest::~IOFWDReadRequest ()
{
   if (param_.mem_starts)
      delete[] param_.mem_starts;
   if (param_.mem_sizes)
      delete[] param_.mem_sizes;
   if (param_.file_starts)
      delete[] param_.file_starts;
   if (param_.file_sizes)
      delete[] param_.file_sizes;
   if (param_.bmi_mem_sizes)
      delete[] param_.bmi_mem_sizes;
   if(param_.op_hint)
      zoidfs::util::ZoidFSHintDestroy(&(param_.op_hint));
   if(bmi_buffer_)
      delete bmi_buffer_;
}

//
// Possible optimizations:
//   * switch to hybrid stack/heap arrays for uint64_t arrays
//
const IOFWDReadRequest::ReqParam & IOFWDReadRequest::decodeParam ()
{
   // get the handle
   process (req_reader_, handle_);

   // get the mem count and sizes
   process (req_reader_, param_.mem_count);
   param_.mem_sizes = new size_t[param_.mem_count];
   process (req_reader_, encoder::EncVarArray(param_.mem_sizes, param_.mem_count));

   // get the file count, sizes, and starts
   process (req_reader_, param_.file_count);
   param_.file_starts = new zoidfs::zoidfs_file_ofs_t[param_.file_count];
   process (req_reader_, encoder::EncVarArray(param_.file_starts, param_.file_count));
   param_.file_sizes = new zoidfs::zoidfs_file_ofs_t[param_.file_count];
   process (req_reader_, encoder::EncVarArray(param_.file_sizes, param_.file_count));

   // get the pipeline size
   process (req_reader_, param_.pipeline_size);
   decodeOpHint (&param_.op_hint);

   // init other param vars
   param_.mem_total_size = 0;

   // allocate buffer for normal mode
    if (param_.pipeline_size == 0)
    {
        char * mem = NULL;
        // compute the total size of the io op
        for(size_t i = 0 ; i < param_.mem_count ; i++)
        {
            param_.mem_total_size += param_.mem_sizes[i];
        }

        // create the bmi buffer
        bmi_buffer_ = new iofwdutil::bmi::BMIBuffer(addr_, iofwdutil::bmi::BMI::ALLOC_RECEIVE);
        bmi_buffer_->resize(param_.mem_total_size);
        mem = static_cast<char *>(bmi_buffer_->get());

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

        param_.mem_starts = new char*[param_.file_count];

#if SIZEOF_SIZE_T != SIZEOF_INT64_T
        param_.bmi_mem_sizes = new bmi_size_t[param_.file_count];
#else
        param_.bmi_mem_sizes = NULL;
#endif

        // setup the mem offset and start buffers
        size_t cur = 0;
        for (size_t i = 0; i < param_.file_count; i++)
        {
            param_.mem_starts[i] = mem + cur;
            param_.mem_sizes[i] = param_.file_sizes[i];
#if SIZEOF_SIZE_T != SIZEOF_INT64_T
            param_.bmi_mem_sizes[i] = param_.mem_sizes[i];
#endif
            cur += param_.file_sizes[i];
        }
    }

   param_.handle = &handle_;
   return param_;
}

void IOFWDReadRequest::sendBuffers(const CBType & cb)
{
#if SIZEOF_SIZE_T == SIZEOF_INT64_T
   /* Send the mem_sizes_ array */
   r_.rbmi_.post_send_list(cb, addr_, reinterpret_cast<void*const*>(param_.mem_starts), reinterpret_cast<const bmi_size_t *>(param_.mem_sizes),
                            param_.mem_count, param_.mem_total_size, bmi_buffer_->bmiType(), tag_, 0);
#else
   /* Send the bmi_mem_sizes_ array */
   r_.rbmi_.post_send_list(cb, addr_, reinterpret_cast<void*const*>(param_.mem_starts), reinterpret_cast<const bmi_size_t *>(param_.bmi_mem_sizes),
                            param_.mem_count, param_.mem_total_size, bmi_buffer_->bmiType(), tag_, 0);
#endif
}

iofwdutil::completion::CompletionID * IOFWDReadRequest::sendPipelineBuffer(iofwdutil::bmi::BMIBuffer * buf, size_t size)
{
   iofwdutil::completion::BMICompletionID * id = new iofwdutil::completion::BMICompletionID ();
   bmires_.postSend (id, addr_, (char *)buf->get(), size, buf->bmiType(), tag_, 0);
   return id;
}


void IOFWDReadRequest::reply(const CBType & cb)
{
   simpleOptReply(cb, getReturnCode(), TSSTART << encoder::EncVarArray(param_.file_sizes, param_.file_count));
}

//===========================================================================
   }
}

