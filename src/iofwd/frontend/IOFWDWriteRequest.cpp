#include "IOFWDWriteRequest.hh"
#include "iofwdutil/bmi/BMIOp.hh"
#include "zoidfs/zoidfs-proto.h"
#include "iofwd_config.h"
#include "zoidfs/util/ZoidFSHints.hh"
#include "src/iofwdutil/transform/IOFWDZLib.hh"

namespace iofwd
{
   namespace frontend
   {
//===========================================================================

IOFWDWriteRequest::~IOFWDWriteRequest ()
{
#ifndef USE_TASK_HA
   if (param_.mem_starts)
      delete [] param_.mem_starts;
   if (param_.mem_sizes)
      delete[] param_.mem_sizes;
   if (param_.file_starts)
      delete[] param_.file_starts;
   if (param_.file_sizes)
      delete[] param_.file_sizes;
   if (param_.bmi_mem_sizes)
      delete[] param_.bmi_mem_sizes;
   // NULL check not really required since freeing a NULL
   // ptr is just doing a return from the function
   if(true == op_hint_compress_enabled)
   {
      if(0 == param_.pipeline_size)
      {
	  delete compressed_mem[0];
	  delete []compressed_mem;
	  delete []userCB_;
	  delete GenTransform;
      }
      else
      {
	  for(int ii = 0; ii < NUM_OUTSTANDING_REQUESTS; ii++)
	      delete compressed_mem[ii];
	  delete []compressed_mem;

	  for(int ii = 0; ii < NUM_OUTSTANDING_REQUESTS; ii++)
	      delete decompressed_mem[ii];
	  delete []decompressed_mem;

	  delete []decompressed_size;

	  delete []userCB_;
	  delete GenTransform;
      }
   }
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
#endif
   if(param_.op_hint)
      zoidfs::util::ZoidFSHintDestroy(&(param_.op_hint));
}

IOFWDWriteRequest::ReqParam & IOFWDWriteRequest::decodeParam ()
{
   // init the handle
   process (req_reader_, handle_);

   // init the mem count and sizes
   process (req_reader_, param_.mem_count);
#ifndef USE_TASK_HA
   param_.mem_sizes = new size_t[param_.mem_count];
#else
   param_.mem_sizes = (h.hamalloc<size_t>(param_.mem_count));
#endif
   process (req_reader_, encoder::EncVarArray(param_.mem_sizes, param_.mem_count));

   // init the file count, sizes, and starts
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

   // get the hint
   decodeOpHint (&(param_.op_hint));

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
   /* keep pipelining enabled by default */
   else
   {
        param_.op_hint_pipeline_enabled = true;
   }

   // Memory param_.op_hint is freed in the class destructor
   char *enable_transform = zoidfs::util::ZoidFSHintGet(&(param_.op_hint), ZOIDFS_TRANSFORM);
   if(NULL != enable_transform)
   {
        char *token = NULL, *saveptr = NULL;

	token = strtok_r(enable_transform, ":", &saveptr);

	if(NULL != token)
	{
	    // The rest of the tokens do not matter --> once you get the compression type
	    // check for the compressed size and that's it
	    if(strcasecmp(token, ZOIDFS_TRANSFORM_ZLIB) == 0)
	    {
		op_hint_compress_enabled = true;
		char *compressed_size_str =
		  zoidfs::util::ZoidFSHintGet(&(param_.op_hint), ZOIDFS_COMPRESSED_SIZE);
		compressed_size = (size_t)atol(compressed_size_str);
	    }
	}
        else
        {
            op_hint_compress_enabled = false;
        }
   }
   /* keep pipelining disabled by default */
   else
   {
        op_hint_compress_enabled = false;
   }

   char * enable_header_stuffing = zoidfs::util::ZoidFSHintGet(&(param_.op_hint), ZOIDFS_ENABLE_HEADER_STUFFING);
   if(enable_header_stuffing)
   {
        if(strcmp(enable_pipeline, ZOIDFS_HINT_ENABLED) == 0)
        {
            op_hint_headstuff_enabled = true;
        }
        else
        {
            op_hint_headstuff_enabled = false;
        }
   }
   /* keep pipelining enabled by default */
   else
   {
        op_hint_headstuff_enabled = true;
   }

   // init the rest of the write request params to 0
   param_.mem_total_size = 0;
   param_.mem_expected_size = 0;

   // init the handle
   param_.handle = &handle_;

   // get the max buffer size from BMI
   r_.rbmi_.get_info(addr_, BMI_CHECK_MAXSIZE, static_cast<void *>(&param_.max_buffer_size));

   return param_;
}

void IOFWDWriteRequest::initRequestParams(ReqParam & p, void * bufferMem)
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
#ifndef USE_TASK_HA
            delete[] param_.mem_sizes;
            param_.mem_sizes = new size_t[param_.file_count];
#else
            h.hafree(p.mem_sizes);
            param_.mem_sizes = (h.hamalloc<size_t>(param_.file_count));
#endif
        }

#ifndef USE_TASK_HA
        param_.mem_starts = new char*[param_.file_count];
#else
        param_.mem_starts = (h.hamalloc<char *>(param_.file_count));
#endif

        // if this is a 32bit system, allocate a mem_size buffer using bmi_size_t
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
        // setup the mem offset buffer
        size_t cur = 0;
        for(size_t i = 0; i < param_.file_count ; i++)
        {
            param_.mem_starts[i] = mem + cur;
            param_.mem_sizes[i] = param_.file_sizes[i];
        // if this is a 32bit system, set the bmi_size_t mem size buffer
#if SIZEOF_SIZE_T != SIZEOF_INT64_T
            param_.bmi_mem_sizes[i] = param_.file_sizes[i];
#endif
            cur += param_.file_sizes[i];
        }

        p = param_;

	if(true == op_hint_compress_enabled)
	{
	    GenTransform = new iofwdutil::transform::ZLib ();

	    compressed_mem = new char* [1];
	    compressed_mem[0] = new char [compressed_size];

	    userCB_ = new CBType[1];
	    userCB_[0] = NULL;

	    user_callbacks = 0;

	    decompressed_mem = NULL;
	    decompressed_size = NULL;
	    callback_mem = NULL;
	    next_slot = 0;
	}

    }
    else
    {
	if(true == op_hint_compress_enabled)
	{
	    GenTransform = new iofwdutil::transform::ZLib ();

	    compressed_mem = new char* [NUM_OUTSTANDING_REQUESTS];
	    for(int ii = 0; ii < NUM_OUTSTANDING_REQUESTS; ii++)
		compressed_mem[ii] = new char [param_.pipeline_size];
	    compressed_size = 0;

	    decompressed_mem = new char* [NUM_OUTSTANDING_REQUESTS];
	    for(int ii = 0; ii < NUM_OUTSTANDING_REQUESTS; ii++)
		decompressed_mem[ii] = new char [param_.pipeline_size<<1];

	    decompressed_size = new size_t [NUM_OUTSTANDING_REQUESTS];
	    for(int ii = 0; ii < NUM_OUTSTANDING_REQUESTS; ii++)
		decompressed_size[ii] = 0;

	    callback_mem = new char* [NUM_OUTSTANDING_REQUESTS];
	    for(int ii = 0; ii < NUM_OUTSTANDING_REQUESTS; ii++)
		callback_mem[ii] = NULL;

	    next_slot = 0;

	    user_callbacks = 0;
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

    /* delete the buffer */
    delete rb->buffer_;
    rb->buffer_ = NULL;
}

void IOFWDWriteRequest::recvComplete(int recvStatus)
{
   unsigned int i = 0;
   int outState = 0;
   size_t outBytes = 0;

   ASSERT(true == op_hint_compress_enabled);
   ASSERT(1 == user_callbacks);

   for(i = 0; i < param_.mem_count; i++)
   {
      GenTransform->transform(compressed_mem[0],
	compressed_size,
	param_.mem_starts[i],
	param_.mem_sizes[i],
	&outBytes,
	&outState,
	false);

      if(iofwdutil::transform::CONSUME_OUTBUF == outState)
      {
	  continue;
      }
      else if(iofwdutil::transform::TRANSFORM_STREAM_END == outState)
      {
	  // This will normally occur only when i = parma_.mem_count
	  // at the end of the decompression
	  // call the user callback stored previously
	  // there should be only one callback
	  userCB_[0](recvStatus);
	  break; // Control will never reach this place
      }
      ASSERT(iofwdutil::transform::SUPPLY_INBUF != outState);
      ASSERT(iofwdutil::transform::TRANSFORM_STREAM_ERROR != outState);
   }
}

void IOFWDWriteRequest::recvBuffers(const CBType & cb, RetrievedBuffer * rb)
{
    param_.mem_expected_size = 0;

    if(false == op_hint_compress_enabled)
    {
#if SIZEOF_SIZE_T == SIZEOF_INT64_T
      r_.rbmi_.post_recv_list(cb, addr_, reinterpret_cast<void*const*>(param_.mem_starts), reinterpret_cast<const bmi_size_t *>(param_.mem_sizes),
			      param_.mem_count, param_.mem_total_size, &(param_.mem_expected_size), dynamic_cast<iofwdutil::mm::BMIMemoryAlloc *>(rb->buffer_)->bmiType(), tag_, 0);
#else
      r_.rbmi_.post_recv_list(cb, addr_, reinterpret_cast<void*const*>(param_.mem_starts), reinterpret_cast<const bmi_size_t*>(param_.bmi_mem_sizes),
			      param_.mem_count, param_.mem_total_size, &(param_.mem_expected_size), dynamic_cast<iofwdutil::mm::BMIMemoryAlloc *>(rb->buffer_)->bmiType(), tag_, 0);
#endif
    }
    else
    {
      CBType transformCB = boost::bind(&IOFWDWriteRequest::recvComplete, boost::ref(*this), _1);

      userCB_[user_callbacks++] = cb;

      ASSERT(1 == user_callbacks);

      STATIC_ASSERT(sizeof(bmi_size_t) == sizeof(size_t));

#if SIZEOF_SIZE_T == SIZEOF_INT64_T
      r_.rbmi_.post_recv_list(transformCB, addr_,
            reinterpret_cast<void*const*>(compressed_mem),
            reinterpret_cast<const bmi_size_t*>(&compressed_size), 1,
            param_.mem_total_size, &(param_.mem_expected_size),
            dynamic_cast<iofwdutil::mm::BMIMemoryAlloc
            *>(rb->buffer_)->bmiType(), tag_, 0);
#else
      r_.rbmi_.post_recv_list(transformCB, addr_, reinterpret_cast<void*const*>(compressed_mem), reinterpret_cast<const bmi_size_t*>(&compressed_size), 1, param_.mem_total_size, &(param_.mem_expected_size), dynamic_cast<iofwdutil::mm::BMIMemoryAlloc *>(rb->buffer_)->bmiType(), tag_, 0);
#endif
    }
}

void IOFWDWriteRequest::recvPipelineComplete(int recvStatus, int my_slot)
{
   int outState = 0;
   size_t outBytes = 0;
   size_t offset = 0;
   size_t prevbytes = 0;
   size_t bytes = 0;
   int iterations = 0;

   ASSERT(true == op_hint_compress_enabled);

   GenTransform->transform(compressed_mem[my_slot],
	param_.pipeline_size,
	decompressed_mem[my_slot],
	param_.pipeline_size<<1,
	&outBytes,
	&outState,
	true);

   ASSERT(iofwdutil::transform::TRANSFORM_STREAM_ERROR != outState);
   ASSERT(outBytes <= (param_.pipeline_size<<1));

   decompressed_size[my_slot] = outBytes;

   if(0 == my_slot)
      offset = prevbytes = bytes = 0;
   else
   {
      prevbytes = decompressed_size[my_slot - 1] % param_.pipeline_size;
      bytes = param_.pipeline_size - prevbytes;
      offset = (decompressed_size[my_slot - 1] / param_.pipeline_size) * param_.pipeline_size;
   }

   if(0 != prevbytes)
      memcpy(callback_mem[next_slot], decompressed_mem[my_slot-1] + offset, prevbytes);

   iterations = (decompressed_size[my_slot] + prevbytes) / param_.pipeline_size;

   for(int ii = 0; ii < iterations; ii++, next_slot++, prevbytes = 0, bytes = param_.pipeline_size)
   {
      memcpy(callback_mem[next_slot]+prevbytes, decompressed_mem[my_slot], bytes);
      userCB_[next_slot](recvStatus);
   }

   if(iofwdutil::transform::SUPPLY_INBUF == outState)
   {
      CBType transformCB = boost::bind(&IOFWDWriteRequest::recvPipelineComplete, boost::ref(*this), _1, my_slot+1);

      r_.rbmi_.post_recv(transformCB,
	  addr_,
	  compressed_mem[my_slot+1],
	  param_.pipeline_size,
	  &(param_.mem_expected_size), 
	  BMI_EXT_ALLOC, tag_, 0);
   }
}

void IOFWDWriteRequest::recvPipelineBufferCB(iofwdevent::CBType cb, RetrievedBuffer * rb, size_t size)
{
   param_.mem_expected_size = 0;

   if(false == op_hint_compress_enabled)
   {
      r_.rbmi_.post_recv(cb, addr_, dynamic_cast<iofwdutil::mm::BMIMemoryAlloc *>(rb->buffer_)->getMemory(), size, &(param_.mem_expected_size), 
	    dynamic_cast<iofwdutil::mm::BMIMemoryAlloc *>(rb->buffer_)->bmiType(), tag_, 0);
   }
   else
   {
      boost::mutex::scoped_lock l (mp_);
      CBType transformCB = boost::bind(&IOFWDWriteRequest::recvPipelineComplete, boost::ref(*this), _1, user_callbacks);
      userCB_[user_callbacks] = cb;
      callback_mem[user_callbacks++] = (char*)(rb->buffer_)->getMemory();

      if(1 == user_callbacks)
	  r_.rbmi_.post_recv(transformCB, addr_, reinterpret_cast<iofwdutil::mm::BMIMemoryAlloc *>(&compressed_mem[user_callbacks-1]), size, &(param_.mem_expected_size), BMI_EXT_ALLOC, tag_, 0);
   }
}

void IOFWDWriteRequest::reply(const CBType & cb)
{
   simpleOptReply(cb, getReturnCode(), TSSTART << encoder::EncVarArray(param_.file_sizes, param_.file_count));
}

//===========================================================================
   }
}
