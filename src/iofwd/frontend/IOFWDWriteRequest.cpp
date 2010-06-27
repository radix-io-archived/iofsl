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
   delete []param_.compressed_mem;
   delete param_.GenTransform;
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
   char *enable_compress = zoidfs::util::ZoidFSHintGet(&(param_.op_hint), ZOIDFS_ENABLE_COMPRESS);
   if(enable_compress)
   {
        if(strcmp(enable_compress, ZOIDFS_HINT_ENABLED) == 0)
        {
            param_.op_hint_compress_enabled = true;
	    char *compressed_size_str = zoidfs::util::ZoidFSHintGet(&(param_.op_hint), ZOIDFS_COMPRESS_SIZE);
	    param_.compressed_size = (size_t)atol(compressed_size_str);
        }
        else
        {
            param_.op_hint_compress_enabled = false;
        }
   }
   /* keep pipelining disabled by default */
   else
   {
        param_.op_hint_compress_enabled = false;
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
	if(true == param_.op_hint_compress_enabled)
	{
	    param_.GenTransform = new iofwdutil::iofwdtransform::ZLib;
	    param_.compressed_mem = new char[param_.compressed_size];
	    if(NULL == param_.compressed_mem)
	      throw "IOFWDWriteRequest::initRequestParams() failed!";
	}

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
    }
    else
    {
	param_.compressed_mem = new char[param_.pipeline_size];
	if(NULL == param_.compressed_mem)
	  throw "IOFWDWriteRequest::initRequestParams() failed (new char [])!";

	//param_.transform_buf = new param_.buf* [1024];
	if(NULL == param_.transform_buf)
	  throw "IOFWDWriteRequest::initRequestParams() failed (new param_.buf [])!";
	for(int ii = 0; ii < 1024; ii++)
	{
	  param_.transform_buf[ii]->buf = NULL;
	  param_.transform_buf[ii]->byte_count = 0;
	}
	param_.transform_consume_buf = 0;
	param_.transform_buf_count = 0;
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

   if(false == param_.op_hint_compress_enabled)
   {
      // You should never have called this call back at all
      throw "IOFWDWriteRequest::recvComplete() failed (Wrong Callback)!";
   }

   for(i = 0; i < param_.mem_count; i++)
   {
      param_.GenTransform->transform(param_.compressed_mem,
	param_.compressed_size,
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
	  param_.UserCB(recvStatus);
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

    if(false == param_.op_hint_compress_enabled)
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

      param_.UserCB = cb;
#if SIZEOF_SIZE_T == SIZEOF_INT64_T
      r_.rbmi_.post_recv_list(transformCB, addr_, reinterpret_cast<void*const*>(param_.compressed_mem), reinterpret_cast<const bmi_size_t *>(param_.compressed_size), 1, param_.mem_total_size, &(param_.mem_expected_size), dynamic_cast<iofwdutil::mm::BMIMemoryAlloc *>(rb->buffer_)->bmiType(), tag_, 0);
#else
      r_.rbmi_.post_recv_list(transformCB, addr_, reinterpret_cast<void*const*>(param_.compressed_mem), reinterpret_cast<const bmi_size_t*>(param_.compressed_size), 1, param_.mem_total_size, &(param_.mem_expected_size), dynamic_cast<iofwdutil::mm::BMIMemoryAlloc *>(rb->buffer_)->bmiType(), tag_, 0);
#endif
    }
}

void IOFWDWriteRequest::recvPipelineComplete(int recvStatus)
{
   int i = 0;
   int outState = 0;
   size_t outBytes = 0;

   if(false == param_.op_hint_compress_enabled)
   {
      // You should never have called this call back at all
      throw "IOFWDWriteRequest::recvComplete() failed (Wrong Callback)!";
   }

decompress:
   if(0 == param_.transform_buf[param_.transform_buf_count]->byte_count)
   {
      // output buffer is new
      param_.transform_buf[param_.transform_buf_count]->buf = new char [param_.pipeline_size];
      if(NULL == param_.transform_buf[param_.transform_buf_count]->buf)
	  throw "IOFWDWriteRequest::initRequestParams() failed (new param_.buf [])!";
      param_.GenTransform->transform(param_.compressed_mem,
	  param_.pipeline_size,
	  param_.transform_buf[param_.transform_buf_count]->buf,
	  param_.pipeline_size,
	  &outBytes,
	  &outState,
	  false);
   }
   if(param_.transform_buf[param_.transform_buf_count]->byte_count < param_.pipeline_size)
   {
      // output buffer is partially filled
      param_.GenTransform->transform(param_.compressed_mem,
	  param_.pipeline_size,
	  param_.transform_buf[param_.transform_buf_count]->buf,
	  param_.pipeline_size-param_.transform_buf[param_.transform_buf_count]->byte_count,
	  &outBytes,
	  &outState,
	  false);
   }

   param_.transform_buf[param_.transform_buf_count]->byte_count += outBytes;
   if(param_.transform_buf[param_.transform_buf_count]->byte_count == param_.pipeline_size)
      param_.transform_buf_count++;

   if(iofwdutil::iofwdtransform::CONSUME_OUTBUF == outState)
   {
      // decompress until the input buffer is completely consumed
      goto decompress;
   }
   else if(iofwdutil::iofwdtransform::TRANSFORM_STREAM_END == outState)
   {
      if(param_.transform_consume_buf == param_.transform_buf_count)
	 return;
      memcpy(param_.transform_mem,
	  param_.transform_buf[param_.transform_consume_buf]->buf,
	  param_.transform_buf[param_.transform_consume_buf]->byte_count);
      delete []param_.transform_buf[param_.transform_consume_buf]->buf;
      param_.transform_consume_buf++;
      param_.UserCB(recvStatus);
   }
   else if(iofwdutil::iofwdtransform::SUPPLY_INBUF == outState)
   {
      if(param_.transform_consume_buf == param_.transform_buf_count)
	 return;
      memcpy(param_.transform_mem,
	  param_.transform_buf[param_.transform_consume_buf]->buf,
	  param_.transform_buf[param_.transform_consume_buf]->byte_count);
      delete []param_.transform_buf[param_.transform_consume_buf]->buf;
      param_.transform_consume_buf++;
      param_.UserCB(recvStatus);
   }
   else if (iofwdutil::iofwdtransform::TRANSFORM_STREAM_ERROR == outState)
   {
      throw "IOFWDWriteRequest::recvComplete() failed (TRANSFORM_STREAM_ERROR in Pipelined Mode)!";
   }
}

void IOFWDWriteRequest::recvPipelineBufferCB(iofwdevent::CBType cb, RetrievedBuffer * rb, size_t size)
{
   param_.mem_expected_size = 0;

   if(false == param_.op_hint_compress_enabled)
   {
      r_.rbmi_.post_recv(cb, addr_, dynamic_cast<iofwdutil::mm::BMIMemoryAlloc *>(rb->buffer_)->getMemory(), size, &(param_.mem_expected_size), 
	    dynamic_cast<iofwdutil::mm::BMIMemoryAlloc *>(rb->buffer_)->bmiType(), tag_, 0);
   }
   else
   {
      CBType transformCB = boost::bind(&IOFWDWriteRequest::recvPipelineComplete, boost::ref(this), _1);

      param_.transform_mem = (char*)rb->buffer_->getMemory();
      param_.UserCB = cb;

      r_.rbmi_.post_recv(transformCB, addr_, reinterpret_cast<iofwdutil::mm::BMIMemoryAlloc *>(param_.compressed_mem), size, &(param_.mem_expected_size), dynamic_cast<iofwdutil::mm::BMIMemoryAlloc *>(rb->buffer_)->bmiType(), tag_, 0);
   }
}

void IOFWDWriteRequest::reply(const CBType & cb)
{
   simpleOptReply(cb, getReturnCode(), TSSTART << encoder::EncVarArray(param_.file_sizes, param_.file_count));
}

//===========================================================================
   }
}

















#if 0
   // This code is more efficient
   // Since we know that param_.mem_sizes
   // is a contiguous buffer in memory
   // you don't loop => just decompress it in one go
   if(false == param_.op_hint_compress_enabled)
   {
decompress_again:
      param_.GenTransform->transform(param_.compressed_mem,
	  param_.compressed_size,
	  param_.mem_starts[0],
	  param_.mem_total_size,
	  &outBytes,
	  &outState,
	  true);

      if(iofwdutil::iofwdtransform::TRANSFORM_STREAM_END == outState)
      {
	  param_.UserCB(recvStatus);
	  goto exit_recvComplete;
      }

      if(iofwdutil::iofwdtransform::CONSUME_OUTBUF == outState)
      {
	  param_.UserCB(recvStatus);
	  goto exit_recvComplete;
      }
      goto decompress_again;
   }
#endif
