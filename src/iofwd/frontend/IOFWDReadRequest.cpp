#include "IOFWDReadRequest.hh"
#include "zoidfs/zoidfs-proto.h"
#include "iofwd_config.h"

#include "iofwdevent/BMIResource.hh"

namespace iofwd
{
   namespace frontend
   {
//===========================================================================

IOFWDReadRequest::~IOFWDReadRequest ()
{
}

//
// Possible optimizations:
//   * switch to hybrid stack/heap arrays for uint64_t arrays
//
IOFWDReadRequest::ReqParam & IOFWDReadRequest::decodeParam ()
{
   // get the handle
   process (req_reader_, handle_);

   // get the mem count and sizes
   process (req_reader_, param_.mem_count);
   param_.mem_sizes.reset(new size_t[param_.mem_count]);
   process (req_reader_, encoder::EncVarArray(param_.mem_sizes.get(), param_.mem_count));

   // get the file count, sizes, and starts
   process (req_reader_, param_.file_count);

   param_.file_starts.reset(new zoidfs::zoidfs_file_ofs_t[param_.file_count]);
   process (req_reader_, encoder::EncVarArray(param_.file_starts.get(), param_.file_count));
   param_.file_sizes.reset(new zoidfs::zoidfs_file_ofs_t[param_.file_count]);
   process (req_reader_, encoder::EncVarArray(param_.file_sizes.get(), param_.file_count));

   // get the pipeline size
   process (req_reader_, param_.pipeline_size);
   param_.op_hint = &op_hint_;
   decodeOpHint((*param_.op_hint)());

   // check for hints here
   int hint_found = 0; 
   char enable_pipeline[32];
   zoidfs::hints::zoidfs_hint_get(*((*param_.op_hint)()), ZOIDFS_ENABLE_PIPELINE, 32, enable_pipeline, &hint_found);
   if(hint_found)
   {
        if(strcmp(enable_pipeline, ZOIDFS_HINT_ENABLED) == 0)
        {
            param_.op_hint_pipeline_enabled = true;
        }
        else
        {
            param_.op_hint_pipeline_enabled = false;
            param_.pipeline_size = 0;
        }
   }
   /* keep pipelining enabled by default */
   else
   {
        param_.op_hint_pipeline_enabled = true;
   }

   // init other param vars
   mem_total_size = 0;

   // get the max buffer size from BMI
   r_.rbmi_.get_info(addr_, BMI_CHECK_MAXSIZE, static_cast<void *>(&param_.max_buffer_size));

   param_.handle = &handle_;
   return param_;
}

void IOFWDReadRequest::initRequestParams(ReqParam & p, void * bufferMem)
{
   // allocate buffer for normal mode
    if (p.pipeline_size == 0)
    {
        char * mem = NULL;
        // compute the total size of the io op
        for(size_t i = 0 ; i < p.mem_count ; i++)
        {
            mem_total_size += p.mem_sizes[i];
        }

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

#if SIZEOF_SIZE_T != SIZEOF_INT64_T
        bmi_mem_sizes.reset(new bmi_size_t[p.file_count]);
#else
        bmi_mem_sizes.reset();
#endif

        // setup the mem offset and start buffers
        size_t cur = 0;
        for (size_t i = 0; i < p.file_count; i++)
        {
            p.mem_starts[i] = mem + cur;
            p.mem_sizes[i] = p.file_sizes[i];
#if SIZEOF_SIZE_T != SIZEOF_INT64_T
            bmi_mem_sizes[i] = p.mem_sizes[i];
#endif
            cur += p.file_sizes[i];
        }
    }

}

void IOFWDReadRequest::sendBuffers(const iofwdevent::CBType & cb, RetrievedBuffer * rb)
{
#if SIZEOF_SIZE_T == SIZEOF_INT64_T
   /* Send the mem_sizes_ array */
   r_.rbmi_.post_send_list(cb, addr_, reinterpret_cast<const
           void*const*>(param_.mem_starts.get()),
           reinterpret_cast<const bmi_size_t *>(param_.mem_sizes.get()),
           param_.mem_count,
           mem_total_size,
           dynamic_cast<iofwdutil::mm::BMIMemoryAlloc *>(rb->buffer_)->bmiType(),
           tag_, 0);
#else
   /* Send the bmi_mem_sizes_ array */
   r_.rbmi_.post_send_list(cb, addr_, reinterpret_cast<const
           void*const*>(param_.mem_starts.get()), reinterpret_cast<const
           bmi_size_t *>(bmi_mem_sizes.get()),
           param_.mem_count,
           mem_total_size,
           dynamic_cast<iofwdutil::mm::BMIMemoryAlloc *>(rb->buffer_)->bmiType(),
           tag_, 0);
#endif
}

void IOFWDReadRequest::sendPipelineBufferCB(const iofwdevent::CBType cb, RetrievedBuffer * rb, size_t size)
{
   r_.rbmi_.post_send(cb, addr_, (const void *)dynamic_cast<iofwdutil::mm::BMIMemoryAlloc *>(rb->buffer_)->getMemory(), size, dynamic_cast<iofwdutil::mm::BMIMemoryAlloc *>(rb->buffer_)->bmiType(), tag_, 0);
}

void IOFWDReadRequest::reply(const CBType & cb)
{
   simpleOptReply(cb, getReturnCode(), TSSTART <<
           encoder::EncVarArray(param_.file_sizes.get(), param_.file_count));
}

void IOFWDReadRequest::allocateBuffer(iofwdevent::CBType cb, RetrievedBuffer * rb)
{
    /* allocate the buffer wrapper */
    rb->buffer_ = new iofwdutil::mm::BMIMemoryAlloc(addr_, iofwdutil::bmi::BMI::ALLOC_SEND, rb->getsize());

    iofwdutil::mm::BMIMemoryManager::instance().alloc(cb, rb->buffer_);
}

void IOFWDReadRequest::releaseBuffer(RetrievedBuffer * rb)
{
    iofwdutil::mm::BMIMemoryManager::instance().dealloc(rb->buffer_);

    delete rb->buffer_;
    rb->buffer_ = NULL;
}

//===========================================================================
   }
}

