#include "IOFWDWriteRequest.hh"
#include "zoidfs/zoidfs-proto.h"
#include "iofwd_config.h"

#include "iofwdevent/BMIResource.hh"


namespace iofwd
{
   namespace frontend
   {
//===========================================================================

/**
 * Deconstructor for IOFWDWriteRequest. 
 */
IOFWDWriteRequest::~IOFWDWriteRequest ()
{
   zoidfs::hints::zoidfs_hint_free(param_.op_hint);
}

IOFWDWriteRequest::ReqParam & IOFWDWriteRequest::decodeParam ()
{

   // init the handle
   process (req_reader_, handle_);

   // init the mem count and sizes
   process (req_reader_, param_.mem_count);
   param_.mem_sizes.reset(new size_t[param_.mem_count]);
   process (req_reader_, encoder::EncVarArray(param_.mem_sizes.get(), param_.mem_count));

   // init the file count, sizes, and starts
   process (req_reader_, param_.file_count);
   param_.file_starts.reset(new zoidfs::zoidfs_file_ofs_t[param_.file_count]);
   process (req_reader_, encoder::EncVarArray(param_.file_starts.get(), param_.file_count));
   param_.file_sizes.reset(new zoidfs::zoidfs_file_ofs_t[param_.file_count]);
   process (req_reader_, encoder::EncVarArray(param_.file_sizes.get(), param_.file_count));

   // get the pipeline size
   process (req_reader_, param_.pipeline_size);

   // get the hint
   zoidfs::hints::zoidfs_hint_create(&op_hint_);
   param_.op_hint = &op_hint_;
   decodeOpHint (param_.op_hint);

   // check for hints here
   int hint_found = 0;
   char enable_pipeline[32];
   zoidfs::hints::zoidfs_hint_get(*(param_.op_hint), ZOIDFS_ENABLE_PIPELINE, 32, enable_pipeline, &hint_found);
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

   // init the rest of the write request params to 0
   param_.mem_total_size = 0;
   mem_expected_size = 0;

   // init the handle
   param_.handle = &handle_;

   // get the max buffer size from BMI
   r_.rbmi_.get_info(addr_, BMI_CHECK_MAXSIZE, static_cast<void *>(&param_.max_buffer_size));

   return param_;
}

void IOFWDWriteRequest::initRequestParams(ReqParam & p, void * bufferMem)
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

        // if this is a 32bit system, allocate a mem_size buffer using bmi_size_t
#if SIZEOF_SIZE_T != SIZEOF_INT64_T
        bmi_mem_sizes.reset(new bmi_size_t[p.file_count]);
#else
        bmi_mem_sizes.reset();
#endif
        // setup the mem offset buffer
        size_t cur = 0;
        for(size_t i = 0; i < p.file_count ; i++)
        {
            p.mem_starts[i] = mem + cur;
            p.mem_sizes[i] = p.file_sizes[i];
        // if this is a 32bit system, set the bmi_size_t mem size buffer
#if SIZEOF_SIZE_T != SIZEOF_INT64_T
            bmi_mem_sizes[i] = p.file_sizes[i];
#endif
            cur += p.file_sizes[i];
        }
    }
}

void IOFWDWriteRequest::allocateBuffer(iofwdevent::CBType cb, RetrievedBuffer * rb)
{
    /* allocate the buffer wrapper */
    rb->buffer_ = new iofwdutil::mm::BMIMemoryAlloc(addr_, iofwdutil::bmi::BMI::ALLOC_RECEIVE, rb->getsize());

    iofwdutil::mm::BMIMemoryManager::instance().alloc(cb, rb->buffer_);
}

void IOFWDWriteRequest::releaseBuffer(RetrievedBuffer * rb)
{
    iofwdutil::mm::BMIMemoryManager::instance().dealloc(rb->buffer_);

    delete rb->buffer_;
    rb->buffer_ = NULL;
}

void IOFWDWriteRequest::recvBuffers(const CBType & cb, RetrievedBuffer * rb)
{
    mem_expected_size = 0;

#if SIZEOF_SIZE_T == SIZEOF_INT64_T
   r_.rbmi_.post_recv_list(cb, addr_,
           reinterpret_cast<void*const*>(param_.mem_starts.get()),
           reinterpret_cast<const bmi_size_t *>(param_.mem_sizes.get()),
           param_.mem_count,
           param_.mem_total_size,
           &(mem_expected_size),
           dynamic_cast<iofwdutil::mm::BMIMemoryAlloc *>(rb->buffer_)->bmiType(),
           tag_, 0);
#else
   r_.rbmi_.post_recv_list(cb, addr_,
           reinterpret_cast<void*const*>(param_.mem_starts.get()),
           reinterpret_cast<const bmi_size_t*>(bmi_mem_sizes.get()),
           param_.mem_count,
           param_.mem_total_size,
           &(mem_expected_size),
           dynamic_cast<iofwdutil::mm::BMIMemoryAlloc *>(rb->buffer_)->bmiType(),
           tag_, 0);
#endif
}

void IOFWDWriteRequest::recvPipelineBufferCB(iofwdevent::CBType cb, RetrievedBuffer * rb, size_t size)
{
   mem_expected_size = 0;
   r_.rbmi_.post_recv(cb, addr_, dynamic_cast<iofwdutil::mm::BMIMemoryAlloc *>(rb->buffer_)->getMemory(), size, &(mem_expected_size), 
        dynamic_cast<iofwdutil::mm::BMIMemoryAlloc *>(rb->buffer_)->bmiType(), tag_, 0);
}

void IOFWDWriteRequest::reply(const CBType & cb)
{
   simpleOptReply(cb, getReturnCode(), TSSTART <<
           encoder::EncVarArray(param_.file_sizes.get(), param_.file_count));
}

//===========================================================================
   }
}
