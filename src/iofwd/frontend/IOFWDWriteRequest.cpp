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
   delete []compressed_mem;
   delete GenTransform;
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
   if(bmi_buffer_)
      delete bmi_buffer_;
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
	    if(strcmp(token, "ZLIB") == 0)
	    {
		op_hint_compress_enabled = true;
		char *compressed_size_str =
		  zoidfs::util::ZoidFSHintGet(&(param_.op_hint), ZOIDFS_COMPRESSED_SIZE);
		compressed_size = (size_t)atol(compressed_size_str);
        printf("Compression size: %i\n",compressed_size);
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
	    GenTransform = new iofwdutil::iofwdtransform::ZLib;

	    compressed_mem = new char* [1];
	    if(NULL == compressed_mem)
	      throw "IOFWDWriteRequest::initRequestParams() new char* [] failed!";

	    compressed_mem[0] = new char [param_.mem_total_size];
	    if(NULL == compressed_mem[0])
	      throw "IOFWDWriteRequest::initRequestParams() new char [] failed!";

	    compressed_mem_count = 0;

	    UserCB = new CBType[1];
	    if(NULL == UserCB)
	      throw "IOFWDWriteRequest::initRequestParams() new CBType[] failed!";

	    UserCB[0] = NULL;

	    user_callbacks = 0;

	    transform_mem = NULL;
	    transform_mem_count = 0;
	}

    }
    else
    {
	if(true == op_hint_compress_enabled)
	{
	    compressed_mem = new char*[16];
	    if(NULL == compressed_mem)
	      throw "IOFWDWriteRequest::initRequestParams() failed (new char* [])!";

	    for(int ii = 0; ii < 16; ii++)
	    {
	      compressed_mem[ii] = new char [param_.pipeline_size];
	      if(NULL == compressed_mem[ii])
		throw "IOFWDWriteRequest::initRequestParams() new char [] failed!";
	    }

	    compressed_mem_consume = 0;
	    compressed_mem_count = 0;

	    transform_mem = new buf [16];
	    if(NULL == transform_mem)
	      throw "IOFWDWriteRequest::initRequestParams() failed (new buf [])!";

	    for(int ii = 0; ii < 16; ii++)
	    {
	      transform_mem[ii].buf = NULL;
	      transform_mem[ii].byte_count = 0;
	    }

	    transform_mem_count = 0;

	    UserCB = new CBType[16];
	    if(NULL == UserCB)
	      throw "IOFWDWriteRequest::initRequestParams() new CBType[] failed!";

	    for(int ii = 0; ii < 16; ii++)
	      UserCB[ii] = NULL;

	    user_callbacks = 0;

	    if(0 != pthread_mutex_init(&imp, NULL))
	      throw "IOFWDWriteRequest::initRequestParams() pthread_mutex_init() failed!";

	    if(0 != pthread_cond_init(&icv, NULL))
	      throw "IOFWDWriteRequest::initRequestParams() pthread_cond_init() failed!";

	    if(0 != pthread_mutex_init(&omp, NULL))
	      throw "IOFWDWriteRequest::initRequestParams() pthread_mutex_init() failed!";

	    if(0 != pthread_cond_init(&ocv, NULL))
	      throw "IOFWDWriteRequest::initRequestParams() pthread_cond_init() failed!";
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
}

void IOFWDWriteRequest::recvComplete(int recvStatus)
{
   int i = 0;
   int outState = 0;
   size_t outBytes = 0;

   fprintf(stderr,"me\n");

   if(false == op_hint_compress_enabled)
   {
      // You should never have called this call back at all
      throw "IOFWDWriteRequest::recvComplete() failed (Wrong Callback)!";
   }
   for(i = 0; i < param_.mem_count; i++)
   {
      GenTransform->transform(compressed_mem[0],
	compressed_size,
	param_.mem_starts[i],
	param_.mem_sizes[i],
	&outBytes,
	&outState,
	false);

      if(iofwdutil::iofwdtransform::CONSUME_OUTBUF == outState)
      {
	  continue;
      }
      else if(iofwdutil::iofwdtransform::TRANSFORM_STREAM_END == outState)
      {
	  // This will normally occur only when i = parma_.mem_count
	  // at the end of the decompression
	  // call the user callback stored previously
	  // there should be only one callback
	  if(user_callbacks > 1)
	    throw "IOFWDWriteRequest::recvComplete() Multiple user call backs in non pipelined case!";
	  UserCB[0](recvStatus);
	  break; // Control will never reach this place
      }
      else if(iofwdutil::iofwdtransform::SUPPLY_INBUF == outState)
      {
	  // In the non-pipelined case, we cannot have this return status
	  throw "IOFWDWriteRequest::recvComplete() failed (SUPPLY_INBUF in Normal Mode)!";
      }
      else if (iofwdutil::iofwdtransform::TRANSFORM_STREAM_ERROR == outState)
      {
	  throw "IOFWDWriteRequest::recvComplete() failed (TRANSFORM_STREAM_ERROR in Normal Mode)!";
      }
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
      CBType transformCB = boost::bind(&IOFWDWriteRequest::recvComplete, boost::ref(this), _1);

      // There should never be more than one call backs in the non-pipelined case
      UserCB[user_callbacks++] = cb;

      if(user_callbacks > 1)
	throw "IOFWDWriteRequest::recvBuffers() Multiple user call backs in non pipelined case!";

#if SIZEOF_SIZE_T == SIZEOF_INT64_T
      r_.rbmi_.post_recv_list(transformCB, addr_, reinterpret_cast<void*const*>(compressed_mem), reinterpret_cast<const bmi_size_t*>(&compressed_size), 1, param_.mem_total_size, &(param_.mem_expected_size), dynamic_cast<iofwdutil::mm::BMIMemoryAlloc *>(rb->buffer_)->bmiType(), tag_, 0);
#else
      r_.rbmi_.post_recv_list(transformCB, addr_, reinterpret_cast<void*const*>(compressed_mem), reinterpret_cast<const bmi_size_t*>(&compressed_size), 1, param_.mem_total_size, &(param_.mem_expected_size), dynamic_cast<iofwdutil::mm::BMIMemoryAlloc *>(rb->buffer_)->bmiType(), tag_, 0);
#endif
    }
}

void IOFWDWriteRequest::dummyPipelineComplete(int recvStatus)
{
   compressed_mem_count++;
}

void IOFWDWriteRequest::recvPipelineComplete(int recvStatus)
{
   int i = 0;
   int outState = 0;
   size_t outBytes = 0;

   if(false == op_hint_compress_enabled)
      throw "IOFWDWriteRequest::recvComplete() failed (Wrong Callback)!";

   compressed_mem_count++;

   while(iofwdutil::iofwdtransform::TRANSFORM_STREAM_END != GenTransform->getDecompressState())
   {
decompress:
      if(0 == transform_mem[transform_mem_count].byte_count)
      {
	  // output buffer is new
	  pthread_mutex_lock(&omp);
	  if(transform_mem_count == user_callbacks)
	    pthread_cond_wait(&ocv, &omp);
	  pthread_mutex_unlock(&omp);

	  GenTransform->transform(compressed_mem[compressed_mem_consume],
	      param_.mem_expected_size,
	      transform_mem[transform_mem_count].buf,
	      param_.pipeline_size,
	      &outBytes,
	      &outState,
	      false);
      }
      else if(transform_mem[transform_mem_count].byte_count < param_.pipeline_size)
      {
	  // output buffer is partially filled
	  GenTransform->transform(compressed_mem[compressed_mem_consume],
	      param_.mem_expected_size,
	      transform_mem[transform_mem_count].buf,
	      param_.pipeline_size-transform_mem[transform_mem_count].byte_count,
	      &outBytes,
	      &outState,
	      false);
      }

      transform_mem[transform_mem_count].byte_count += outBytes;
      if(transform_mem[transform_mem_count].byte_count == param_.pipeline_size)
	  transform_mem_count++;

      if(iofwdutil::iofwdtransform::CONSUME_OUTBUF == outState)
      {
	  goto decompress;
      }
      else if(iofwdutil::iofwdtransform::TRANSFORM_STREAM_END == outState)
      {
	break;
      }
      else if(iofwdutil::iofwdtransform::SUPPLY_INBUF == outState)
      {
	// Supply new input data
	// In the last packet it may or may not be param_.pipeline_size
	// find out how the code knows about this size
	// assume for now that even if the last packet contains
	// less than param_.pipeline_size bytes, the sender fills it with zeros
	// In order to avoid possible recursion, use a dummy call back rather than yourself
	CBType dummyCB = boost::bind(&IOFWDWriteRequest::dummyPipelineComplete, boost::ref(this), _1);

	r_.rbmi_.post_recv(dummyCB, addr_, reinterpret_cast<iofwdutil::mm::BMIMemoryAlloc *>(compressed_mem[compressed_mem_count++]), param_.pipeline_size, &(param_.mem_expected_size), BMI_EXT_ALLOC, tag_, 0);
      }
      else if (iofwdutil::iofwdtransform::TRANSFORM_STREAM_ERROR == outState)
      {
	  // There's no easy way out ;-)
	  throw "IOFWDWriteRequest::recvComplete() failed (TRANSFORM_STREAM_ERROR in Pipelined Mode)!";
      }
   }

   if(iofwdutil::iofwdtransform::TRANSFORM_STREAM_END == GenTransform->getDecompressState())
   {
      if(transform_mem_count != user_callbacks)
      {
	throw "IOFWDWriteRequest::recvPipelineComplete() transform_mem_count != user_callbacks";
      }

      // decompressed data has already been copied into
      // the o/p buffers specified by the user
      for(int ii = 0; ii < transform_mem_count; ii++)
	UserCB[ii](recvStatus);
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
      CBType transformCB = boost::bind(&IOFWDWriteRequest::recvPipelineComplete, boost::ref(this), _1);

      transform_mem[user_callbacks].buf = (char*)rb->buffer_->getMemory();
      transform_mem[user_callbacks].byte_count = 0;
      UserCB[user_callbacks] = cb;
      user_callbacks++;

      // call post_recv only for the first user call back
      if(1 == user_callbacks)
	  r_.rbmi_.post_recv(transformCB, addr_, reinterpret_cast<iofwdutil::mm::BMIMemoryAlloc *>(compressed_mem[0]), size, &(param_.mem_expected_size), BMI_EXT_ALLOC, tag_, 0);
   }
}

void IOFWDWriteRequest::reply(const CBType & cb)
{
   simpleOptReply(cb, getReturnCode(), TSSTART << encoder::EncVarArray(param_.file_sizes, param_.file_count));
}

//===========================================================================
   }
}
