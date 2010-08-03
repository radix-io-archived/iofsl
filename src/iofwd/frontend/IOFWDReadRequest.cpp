#include "IOFWDReadRequest.hh"
#include "iofwdutil/bmi/BMIOp.hh"
#include "zoidfs/zoidfs-proto.h"
#include "iofwd_config.h"
#include "zoidfs/util/ZoidFSHints.hh"
#include "src/iofwdutil/IOFWDLog.hh"

namespace iofwd
{
   namespace frontend
   {
//===========================================================================

IOFWDReadRequest::~IOFWDReadRequest ()
{
   ZLOG_AUTOTRACE_DEFAULT;

#ifndef USE_TASK_HA
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

   if(NULL != compressed_mem_)
   {
      delete []compressed_mem_[0];
      compressed_mem_[0] = NULL;
   }

   delete []compressed_mem_;
   compressed_mem_ = NULL;

#else
   if (param_.mem_starts)
      h.hafree(param_.mem_starts);
   if (param_.mem_sizes)
      h.hafree(param_.mem_sizes);
   if (param_.file_starts)
      h.hafree(param_.file_starts);
   if (param_.file_sizes)
      h.hafree(param_.file_sizes);
   if (param_.bmi_mem_sizes)
      h.hafree(param_.bmi_mem_sizes);

   if(NULL != compressed_mem_)
   {
      h.hafree(compressed_mem_[0]);
      compressed_mem_[0] = NULL;
   }

   h.hafree(compressed_mem_);
   compressed_mem_ = NULL;
#endif

   if(param_.op_hint)
      zoidfs::util::ZoidFSHintDestroy(&(param_.op_hint));
}

//
// Possible optimizations:
//   * switch to hybrid stack/heap arrays for uint64_t arrays
//
IOFWDReadRequest::ReqParam & IOFWDReadRequest::decodeParam ()
{
   ZLOG_AUTOTRACE_DEFAULT;

   // get the handle
   process (req_reader_, handle_);

   // get the mem count and sizes
   process (req_reader_, param_.mem_count);
#ifndef USE_TASK_HA
   param_.mem_sizes = new size_t[param_.mem_count];
#else
   param_.mem_sizes = (h.hamalloc<size_t>(param_.mem_count));
#endif
   process (req_reader_, encoder::EncVarArray(param_.mem_sizes, param_.mem_count));

   // get the file count, sizes, and starts
   process (req_reader_, param_.file_count);

#ifndef USE_TASK_HA
   param_.file_starts = new zoidfs::zoidfs_file_ofs_t[param_.file_count];
#else
   param_.file_starts = (h.hamalloc<zoidfs::zoidfs_file_ofs_t>(param_.file_count));
#endif
   process (req_reader_, encoder::EncVarArray(param_.file_starts, param_.file_count));
#ifndef USE_TASK_HA
   param_.file_sizes = new zoidfs::zoidfs_file_ofs_t[param_.file_count];
#else
   param_.file_sizes = (h.malloc<zoidfs::zoidfs_file_ofs_t>(param_.file_count));
#endif
   process (req_reader_, encoder::EncVarArray(param_.file_sizes, param_.file_count));

   // get the pipeline size
   process (req_reader_, param_.pipeline_size);
   decodeOpHint (&param_.op_hint);

   // check for hints here
   char * enable_pipeline = zoidfs::util::ZoidFSHintGet(&(param_.op_hint), ZOIDFS_ENABLE_PIPELINE);
   if(enable_pipeline)
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

   char *enable_transform = zoidfs::util::ZoidFSHintGet(&(param_.op_hint),
         ZOIDFS_TRANSFORM);
   if(NULL != enable_transform)
   {
        char *token = NULL, *saveptr = NULL;

        token = strtok_r(enable_transform, ":", &saveptr);

        if(NULL != token)
        {
           transform_.reset
              (iofwdutil::transform::GenericTransformEncodeFactory::construct 
                 (token)());
	   op_hint_compress_enabled_ = true;
        }
   }

   char *enable_crc = zoidfs::util::ZoidFSHintGet(&(param_.op_hint),
       ZOIDFS_CRC);
   if(NULL != enable_crc)
   {
        char *token = NULL, *saveptr = NULL;

        token = strtok_r(enable_crc, ":", &saveptr);

        if(NULL != token)
        {
	    hashFunc_.reset (HashFactory::instance().getHash(token));
	    op_hint_crc_enabled_ = true;
        }
   }

   // init other param vars
   param_.mem_total_size = 0;

   // get the max buffer size from BMI
   r_.rbmi_.get_info(addr_, BMI_CHECK_MAXSIZE, static_cast<void *>(&param_.max_buffer_size));

   param_.handle = &handle_;

   if(true == op_hint_compress_enabled_)
   {
      param_.mem_total_size = 0;
      for(size_t ii = 0; ii < param_.mem_count; ii++)
	param_.mem_total_size += param_.mem_sizes[ii];

      if(0 == param_.pipeline_size)
      {
#ifndef USE_TASK_HA
	  compressed_mem_ = new char* [1];
#else
	  compressed_mem_ = (h.hamalloc<char*> (1));
#endif

#ifndef USE_TASK_HA
	  compressed_mem_[0] = new char [param_.mem_total_size];
#else
	  compressed_mem_[0] = (h.hamalloc<char> (param_.mem_total_size));
#endif
      }
      else
      {
	size_t offset = 0;
	char   *compmem = NULL;

	pipeline_ops_ = param_.mem_total_size / param_.pipeline_size;
	rem_bytes_ = param_.mem_total_size % param_.pipeline_size;

	if(rem_bytes_ > 0)
	{
	  pipeline_ops_++;
	}

#ifndef USE_TASK_HA
	compressed_mem_ = new char* [pipeline_ops_];
#else
	compressed_mem_ = (h.hamalloc<char*>(pipeline_ops_));
#endif

#ifndef USE_TASK_HA
	compmem = new char [param_.mem_total_size];
#else
	compmem = (h.hamalloc<char> (param_.mem_total_size));
#endif

	for(size_t ii = 0; ii < pipeline_ops_; ii++)
	{
	  compressed_mem_[ii] = compmem + offset;
	  offset += param_.pipeline_size;
	}
      }
   }

   return param_;
}

void IOFWDReadRequest::initRequestParams(ReqParam & p, void * bufferMem)
{
    ZLOG_AUTOTRACE_DEFAULT;

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
        mem = static_cast<char *>(bufferMem);

        // NOTICE: mem_starts_ and mem_sizes_ are alignend with file_sizes
        // This is for request scheduler to easily handle the ranges without
        // extra memory copying.

        // only going to reallocate if file and mem counts are diff
        if(param_.mem_count != param_.file_count)
        {
            param_.mem_count = param_.file_count;
#ifndef USE_TASK_HA
            delete[] param_.mem_sizes;
            param_.mem_sizes = new size_t[param_.file_count];
#else
            h.hafree(p.mem_sizes);
            param_.mem_sizes = (h.hamalloc<size_t>(param_.file_count));
#endif
        }

#ifndef USE_TASK_HA
        param_.mem_starts = new void*[param_.file_count];
#else
        param_.mem_starts = (h.hamalloc<char *>(param_.file_count));
#endif

#ifndef USE_TASK_HA
#if SIZEOF_SIZE_T != SIZEOF_INT64_T
        param_.bmi_mem_sizes = new bmi_size_t[param_.file_count];
#else
        param_.bmi_mem_sizes = NULL;
#endif
#else
#if SIZEOF_SIZE_T != SIZEOF_INT64_T
        param_.bmi_mem_sizes = (h.hamalloc<bmi_size_t>(param_.file_count));
#else
        param_.bmi_mem_sizes = NULL;
#endif
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
        p = param_;
    }

}

void IOFWDReadRequest::sendBuffers(const iofwdevent::CBType & cb, RetrievedBuffer * rb)
{
   ZLOG_AUTOTRACE_DEFAULT;

   if(false == op_hint_compress_enabled_)
   {
      if(true == op_hint_crc_enabled_)
      {
	for(size_t ii = 0; ii < param_.mem_count; ii++)
	{
#if SIZEOF_SIZE_T == SIZEOF_INT64_T
	  hashFunc_->process (param_.mem_starts[ii], param_.mem_sizes[ii]);
#else
	  hashFunc_->process (param_.mem_starts[ii], param_.bmi_mem_sizes[ii]);
#endif
	}
      }

#if SIZEOF_SIZE_T == SIZEOF_INT64_T
      /* Send the mem_sizes_ array */
      r_.rbmi_.post_send_list(cb,
	  addr_,
	  reinterpret_cast<const void*const*>(param_.mem_starts),
	  reinterpret_cast<const bmi_size_t *>(param_.mem_sizes),
	  param_.mem_count,
	  param_.mem_total_size,
	  dynamic_cast<iofwdutil::mm::BMIMemoryAlloc *>(rb->buffer_)->bmiType(),
	  tag_, 0);
#else
      /* Send the bmi_mem_sizes_ array */
      r_.rbmi_.post_send_list(cb,
	  addr_,
	  reinterpret_cast<const void*const*>(param_.mem_starts),
	  reinterpret_cast<const bmi_size_t *>(param_.bmi_mem_sizes),
	  param_.mem_count,
	  param_.mem_total_size,
	  dynamic_cast<iofwdutil::mm::BMIMemoryAlloc *>(rb->buffer_)->bmiType(),
	  tag_, 0);
#endif
   }
   else
   {
      int    outState = 0;
      size_t outBytes = 0;
      bool   flushFlag = false;

      for(size_t ii = 0; ii < param_.mem_count; ii++)
      {
	  flushFlag = (ii == param_.mem_count - 1) ? true : false;

	  transform_->transform(param_.mem_starts[ii],
#if SIZEOF_SIZE_T == SIZEOF_INT64_T
	    param_.mem_sizes[ii],
#else
	    param_.bmi_mem_sizes[ii],
#endif
	    compressed_mem_[0]+compressed_size_,
	    param_.mem_total_size-compressed_size_,
	    &outBytes,
	    &outState,
	    flushFlag);

	  compressed_size_ += outBytes;

	  ASSERT(outBytes <= param_.mem_total_size-compressed_size_);
	  ASSERT(iofwdutil::transform::CONSUME_OUTBUF != outState);
      }

      if(true == op_hint_crc_enabled_)
	hashFunc_->process (compressed_mem_[0], compressed_size_);

      r_.rbmi_.post_send_list(cb,
	  addr_,
	  reinterpret_cast<const void*const*>(compressed_mem_),
	  reinterpret_cast<const bmi_size_t *>(&compressed_size_),
	  1,
	  compressed_size_,
	  BMI_EXT_ALLOC,
	  tag_, 0);
   }
}

void IOFWDReadRequest::sendPipelineBufferCB(const iofwdevent::CBType cb, RetrievedBuffer * rb, size_t size)
{
   ZLOG_AUTOTRACE_DEFAULT;

   if(false == op_hint_compress_enabled_)
   {
      if(true == op_hint_crc_enabled_)
	hashFunc_->process (rb->buffer_, size);

      r_.rbmi_.post_send(cb,
	  addr_,
	  (const void *)dynamic_cast<iofwdutil::mm::BMIMemoryAlloc *>(rb->buffer_)->getMemory(),
	  size,
	  dynamic_cast<iofwdutil::mm::BMIMemoryAlloc *>(rb->buffer_)->bmiType(),
	  tag_, 0);
   }
   else
   {
      int    outState = 0;
      int    recvStatus = 0;
      size_t outBytes = 0;
      size_t bytes = 0;
      size_t pipelines_posted = 0;
      bool   flush_flag = false;

      boost::mutex::scoped_lock l (mp_);

      user_callbacks_++;

      flush_flag = (user_callbacks_ == pipeline_ops_) ? true : false;

      transform_->transform(rb->buffer_->getMemory(),
	size,
	compressed_mem_[0]+compressed_size_,
	param_.mem_total_size-compressed_size_,
	&outBytes,
	&outState,
	flush_flag);

      ASSERT(outBytes <= param_.mem_total_size-compressed_size_);

      if(true == op_hint_crc_enabled_ && 0 != outBytes)
	  hashFunc_->process (compressed_mem_[0]+compressed_size_, outBytes);

      compressed_size_ += outBytes;

      pipelines_posted = compressed_size_ / param_.pipeline_size;

      if(rem_bytes_ > 0 && true == flush_flag)
      {
	  pipelines_posted++;
      }

      if(pipelines_posted > 0)
      {
	  while(next_slot_ < std::min(user_callbacks_, pipelines_posted))
	  {
	      bytes = param_.pipeline_size;

	      if(true == flush_flag)
	      {
		if(next_slot_ == pipelines_posted - 1)
		  bytes = rem_bytes_;
	      }

	      r_.rbmi_.post_send(cb,
		  addr_,
		  compressed_mem_[next_slot_],
		  bytes,
		  BMI_EXT_ALLOC,
		  tag_, 0);

	      next_slot_++;
	  }
      }
      else
      {
	  cb(recvStatus);
      }
   }
}

void IOFWDReadRequest::reply(const CBType & cb)
{
   ZLOG_AUTOTRACE_DEFAULT;

   int  key_len = strlen(ZOIDFS_CRC);
   char *key = new char[key_len + 1];
   int  value_len = hashFunc_->getHashSize();
   char *value (new char[value_len + 1]);
   zoidfs::zoidfs_op_hint_t *op_hint = zoidfs::util::ZoidFSHintInit(1);

   strcpy(key, ZOIDFS_CRC);
   zoidfs::util::ZoidFSHintAdd(&op_hint, key, value, value_len, ZOIDFS_HINTS_ZC);

   simpleOptReply(cb, getReturnCode(), TSSTART << encoder::EncVarArray(param_.file_sizes, param_.file_count));

   delete []key;
   key = NULL;

   delete []value;
   value = NULL;
}

void IOFWDReadRequest::allocateBuffer(iofwdevent::CBType cb, RetrievedBuffer * rb)
{
    ZLOG_AUTOTRACE_DEFAULT;

    /* allocate the buffer wrapper */
    rb->buffer_ = new iofwdutil::mm::BMIMemoryAlloc(addr_, iofwdutil::bmi::BMI::ALLOC_SEND, rb->getsize());

    iofwdutil::mm::BMIMemoryManager::instance().alloc(cb, rb->buffer_);
}

void IOFWDReadRequest::releaseBuffer(RetrievedBuffer * rb)
{
    ZLOG_AUTOTRACE_DEFAULT;

    iofwdutil::mm::BMIMemoryManager::instance().dealloc(rb->buffer_);

    delete rb->buffer_;
    rb->buffer_ = NULL;
}

//===========================================================================
   }
}

