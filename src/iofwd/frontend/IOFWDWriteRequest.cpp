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
          compressed_mem[0] = NULL;

          delete []compressed_mem;
          compressed_mem = NULL;

          delete []userCB_;
          userCB_ = NULL;

      }
      else
      {
          for(int ii = 0; ii < NUM_OUTSTANDING_REQUESTS; ii++)
          {
              delete compressed_mem[ii];
              compressed_mem[ii] = NULL;
          }
          delete []compressed_mem;
          compressed_mem = NULL;

          delete []decompressed_mem;
          decompressed_mem = NULL;

          delete []userCB_;
          userCB_ = NULL;

          fprintf(stderr, "\n\n");
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
   param_.mem_count = 0;

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

   compressed_mem = NULL;
   compressed_size = 0;
   decompressedBufSize = 0;
   decompressed_mem = NULL;
   decompressed_size = 0;
   callback_mem = NULL;
   userCB_ = NULL;
   next_slot = 0;
   user_callbacks = 0;
   op_hint_compress_enabled = false;
   op_hint_headstuff_enabled = false;
   mem_slot = 0;
   mem_slot_bytes = 0;
   size_of_stuffed_data = 0;

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
        param_.op_hint_pipeline_enabled = false;
   }

   // Memory param_.op_hint is freed in the class destructor
   char *enable_transform = zoidfs::util::ZoidFSHintGet(&(param_.op_hint), ZOIDFS_TRANSFORM);
   if(NULL != enable_transform)
   {
        char *token = NULL, *saveptr = NULL;

        token = strtok_r(enable_transform, ":", &saveptr);

        if(NULL != token)
        {
           transform_.reset
              (iofwdutil::transform::GenericTransformFactory::construct(token)());
           op_hint_compress_enabled = true;
           char *compressed_size_str =
                  zoidfs::util::ZoidFSHintGet(&(param_.op_hint), ZOIDFS_COMPRESSED_SIZE);
	   compressed_size = (size_t)atol(compressed_size_str);
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

   size_t uncompressed_bytes = 0;
   char *enable_header_stuffing = zoidfs::util::ZoidFSHintGet(&(param_.op_hint), ZOIDFS_HEADER_STUFFING);
   if(enable_header_stuffing)
   {
        char *token = NULL, *saveptr = NULL;

        token = strtok_r(enable_header_stuffing, ":", &saveptr);

        if(NULL != token)
            uncompressed_bytes = (size_t)atol(token);

        if(0 == uncompressed_bytes)
          op_hint_headstuff_enabled = false;
        else
          op_hint_headstuff_enabled = true;
   }
   /* keep pipelining enabled by default */
   else
   {
        op_hint_headstuff_enabled = false;
   }

   // init the rest of the write request params to 0
   param_.mem_total_size = 0;
   param_.mem_expected_size = 0;

   // init the handle
   param_.handle = &handle_;

   // get the max buffer size from BMI
   r_.rbmi_.get_info(addr_, BMI_CHECK_MAXSIZE, static_cast<void *>(&param_.max_buffer_size));

   if(0 != param_.pipeline_size && true == op_hint_compress_enabled)
   {
        compressed_mem = new char* [NUM_OUTSTANDING_REQUESTS];
        for(int ii = 0; ii < NUM_OUTSTANDING_REQUESTS; ii++)
            compressed_mem[ii] = new char [param_.pipeline_size];
        compressed_size = 0;

        decompressedBufSize = 0;
        for(unsigned int ii = 0; ii < param_.mem_count; ii++)
          decompressedBufSize += param_.mem_sizes[ii];

        decompressed_mem = new char [decompressedBufSize];
        fprintf(stderr, "++++++++++++ [T%x] %s:%d(%s) decompressedBufSize = %lu\n", (unsigned int)pthread_self(), __FILE__, __LINE__, __func__, decompressedBufSize);

        decompressed_size = 0;

        callback_mem = new char* [NUM_OUTSTANDING_REQUESTS];
        for(int ii = 0; ii < NUM_OUTSTANDING_REQUESTS; ii++)
            callback_mem[ii] = NULL;

        userCB_ = new CBType [NUM_OUTSTANDING_REQUESTS];
        for(int ii = 0; ii < NUM_OUTSTANDING_REQUESTS; ii++)
          userCB_[ii] = NULL;

        next_slot = 0;
        user_callbacks = 0;

        fprintf(stderr, "++++++++++++ [T%x] %s:%d(%s) pipeline_size = %d\n", (unsigned int)pthread_self(), __FILE__, __LINE__, __func__, (int)param_.pipeline_size);
   }

   return param_;
}

void IOFWDWriteRequest::initRequestParams(ReqParam & p, void * bufferMem)
{
    // allocate buffer for normal mode
    if (param_.pipeline_size == 0)
    {
        char * mem = NULL;
        for(size_t i = 0 ; i < param_.file_count ; i++)
        {
            param_.mem_total_size += param_.file_sizes[i];
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
            compressed_mem = new char* [1];
            compressed_mem[0] = new char [compressed_size];
        }

	userCB_ = new CBType[1];
	userCB_[0] = NULL;

        if(true == op_hint_headstuff_enabled)
        {
          int size_of_packet = raw_request_.size();
          void *ptr_to_header = raw_request_.get();
          int size_of_header = req_reader_.getPos();
          unsigned int ii = 0;
          size_t bytes = 0, total_bytes = 0;
          char *position = NULL;
          int outState = 0;
          size_t outBytes = 0;

          size_of_stuffed_data = size_of_packet - size_of_header;
          position = (char*)ptr_to_header + size_of_header;

          fprintf(stderr, "size_of_packet = %d\n", size_of_packet);
          fprintf(stderr, "ptr_to_header = %p\n", ptr_to_header);
          fprintf(stderr, "size_of_header = %d\n", size_of_header);
          fprintf(stderr, "size_of_stuffed_data = %u\n", (unsigned)size_of_stuffed_data);

	  compressed_mem = new char* [1];
	  compressed_size = param_.mem_total_size - size_of_stuffed_data;
	  compressed_mem[0] = new char [compressed_size];

          if(false == op_hint_compress_enabled)
          {
            for(ii = 0;
                ii < param_.mem_count && total_bytes < size_of_stuffed_data;
                ii++, position += bytes, total_bytes += bytes)
            {
                bytes = std::min(param_.mem_sizes[ii], size_of_stuffed_data-total_bytes);
                memcpy(param_.mem_starts[ii], position, bytes);
            }
            mem_slot = ii - 1;
            mem_slot_bytes = bytes;
          }
          else
          {
            for(ii = 0;
                ii < param_.mem_count;
                ii++)
            {
                transform_->transform(position,
                  size_of_stuffed_data,
                  param_.mem_starts[ii],
                  param_.mem_sizes[ii],
                  &outBytes,
                  &outState,
                  true);

                total_bytes += outBytes;
                mem_slot_bytes = outBytes;

                //ASSERT(iofwdutil::transform::TRANSFORM_STREAM_ERROR != outState);

                if(iofwdutil::transform::CONSUME_OUTBUF == outState)
                {
                    mem_slot = ii + 1;
                    continue;
                }
                else if(iofwdutil::transform::SUPPLY_INBUF == outState)
                {
                    mem_slot = ii;
                    break;
                }
                else if(iofwdutil::transform::TRANSFORM_STREAM_END == outState)
                {
                    mem_slot = ii;
                    break;
                }
            }
            size_of_stuffed_data = total_bytes;
          }
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

   ASSERT(1 == user_callbacks);
   ASSERT(size_of_stuffed_data < param_.mem_total_size);

   if(true == op_hint_compress_enabled)
   {
      for(i = mem_slot; i < param_.mem_count; i++)
      {
	  if(i == mem_slot)
	  {
	      transform_->transform(compressed_mem[0],
		compressed_size,
		param_.mem_starts[i]+mem_slot_bytes,
		param_.mem_sizes[i]-mem_slot_bytes,
		&outBytes,
		&outState,
		false);
	  }
	  else
	  {
	      transform_->transform(compressed_mem[0],
		compressed_size,
		param_.mem_starts[i],
		param_.mem_sizes[i],
		&outBytes,
		&outState,
		false);
	  }

	  if(iofwdutil::transform::CONSUME_OUTBUF == outState)
	  {
	      continue;
	  }
	  else if(iofwdutil::transform::TRANSFORM_STREAM_END == outState)
	  {
	      userCB_[0](recvStatus);
	      break;
	  }
	  ASSERT(iofwdutil::transform::SUPPLY_INBUF != outState);
      }
   }
   else
   {
      size_t position = mem_slot_bytes;

      memcpy(param_.mem_starts[mem_slot]+position, compressed_mem[0],
	  param_.mem_sizes[mem_slot]-mem_slot_bytes);
      position += param_.mem_sizes[mem_slot] - mem_slot_bytes;

      for(i = mem_slot+1; i < param_.mem_count; i++)
      {
	  memcpy(param_.mem_starts[i], compressed_mem[0]+position, param_.mem_sizes[i]);
	  position += param_.mem_sizes[i];
      }

      userCB_[0](recvStatus);
   }
}

void IOFWDWriteRequest::recvBuffers(const CBType & cb, RetrievedBuffer * rb)
{
    int recvStatus = 0;

    param_.mem_expected_size = 0;

    if(false == op_hint_compress_enabled && false == op_hint_headstuff_enabled)
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
      CBType transformCB = boost::bind(&IOFWDWriteRequest::recvComplete,
            boost::ref(*this), _1);

      userCB_[user_callbacks++] = cb;

      ASSERT(1 == user_callbacks);

      STATIC_ASSERT(sizeof(bmi_size_t) == sizeof(size_t));

#if SIZEOF_SIZE_T == SIZEOF_INT64_T
      if(size_of_stuffed_data < param_.mem_total_size)
      {
          r_.rbmi_.post_recv_list(transformCB,
              addr_,
              reinterpret_cast<void*const*>(compressed_mem),
              reinterpret_cast<const bmi_size_t*>(&compressed_size),
              1,
              param_.mem_total_size-size_of_stuffed_data,
              &(param_.mem_expected_size),
              BMI_EXT_ALLOC,
              tag_, 0);
      }
      else
      {
          userCB_[0](recvStatus);
      }
#else
      if(size_of_stuffed_data < param_.mem_total_size)
      {
          r_.rbmi_.post_recv_list(transformCB,
              addr_,
              reinterpret_cast<void*const*>(compressed_mem),
              reinterpret_cast<const bmi_size_t*>(&compressed_size),
              1,
              param_.mem_total_size,
              &(param_.mem_expected_size),
              dynamic_cast<iofwdutil::mm::BMIMemoryAlloc *>(rb->buffer_)->bmiType(),
              tag_, 0);
      }
      else
      {
          userCB_[0](recvStatus);
      }
#endif
    }
}

void IOFWDWriteRequest::recvPipelineComplete(int recvStatus, int my_slot)
{
   int outState = 0;
   size_t outBytes = 0;
   size_t bytes = 0;
   size_t remBytes = 0;
   size_t offset = 0;
   int pipelines_ready_to_callback = 0;
   bool partial_slot = false;

   if(iofwdutil::transform::SUPPLY_INBUF == transform_->getTransformState())
   {
      transform_->transform(compressed_mem[my_slot],
            param_.pipeline_size,
            decompressed_mem+decompressed_size,
            decompressedBufSize-decompressed_size,
            &outBytes,
            &outState,
            true);

      ASSERT(iofwdutil::transform::CONSUME_OUTBUF != outState);
      //ASSERT(iofwdutil::transform::TRANSFORM_STREAM_ERROR != outState);
      ASSERT(outBytes <= (decompressedBufSize - decompressed_size));

      decompressed_size += outBytes;
   }

   {
      boost::mutex::scoped_lock l (mp_);
      pipelines_ready_to_callback = (decompressed_size - next_slot*param_.pipeline_size) / param_.pipeline_size;
      if(iofwdutil::transform::TRANSFORM_STREAM_END == outState)
      {
        remBytes = decompressed_size % param_.pipeline_size;
        if(remBytes > 0 && remBytes < param_.pipeline_size)
        {
          pipelines_ready_to_callback++;
          partial_slot = true;
        }
      }
   }

   fprintf(stderr, "============ [T%x]%s:%d(%s) pipelines_ready_to_callback = %d\n", (unsigned int)pthread_self(), __FILE__, __LINE__, __func__,
        pipelines_ready_to_callback);

   if(pipelines_ready_to_callback > 0)
   {
      boost::mutex::scoped_lock l (mp_);
      for(int jj = next_slot; jj < std::min(user_callbacks, pipelines_ready_to_callback); next_slot = ++jj)
      {
          offset = jj * param_.pipeline_size;
          bytes = param_.pipeline_size;
          if(true == partial_slot && jj == pipelines_ready_to_callback)
            bytes = remBytes;
          fprintf(stderr, "============ [T%x]%s:%d(%s) User Callback = %d Data Size = %d\n", (unsigned int)pthread_self(), __FILE__, __LINE__, __func__, jj, (int)bytes);
          memcpy(callback_mem[jj], decompressed_mem+offset, bytes);
          userCB_[jj](recvStatus);
          fprintf(stderr, "============ [T%x]%s:%d(%s) next_slot = %d\n", (unsigned int)pthread_self(), __FILE__, __LINE__, __func__, next_slot);
          fprintf(stderr, "============ [T%x]%s:%d(%s) user_callbacks = %d\n", (unsigned int)pthread_self(), __FILE__, __LINE__, __func__, user_callbacks);
      }
   }

   if(iofwdutil::transform::SUPPLY_INBUF == outState)
   {
      CBType transformCB = boost::bind(&IOFWDWriteRequest::recvPipelineComplete, boost::ref(*this), _1, my_slot+1);

      fprintf(stderr, "============ [T%x]%s:%d(%s) Posting r_.rbmi_.post_recv() %d\n", (unsigned int)pthread_self(), __FILE__, __LINE__, __func__, my_slot+1);
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
   int total_slots = 0;
   int recvStatus = 0;
   size_t offset = 0;
   size_t bytes = 0;
   size_t remBytes = 0;
   bool partial_slot = false;

   param_.mem_expected_size = 0;

   if(false == op_hint_compress_enabled)
   {
      r_.rbmi_.post_recv(cb, addr_, dynamic_cast<iofwdutil::mm::BMIMemoryAlloc *>(rb->buffer_)->getMemory(), size, &(param_.mem_expected_size), 
            dynamic_cast<iofwdutil::mm::BMIMemoryAlloc *>(rb->buffer_)->bmiType(), tag_, 0);
   }
   else
   {
      int callback = 0;
      CBType transformCB;
      {
          boost::mutex::scoped_lock l (mp_);
          transformCB = boost::bind(&IOFWDWriteRequest::recvPipelineComplete, boost::ref(*this), _1, user_callbacks);
          userCB_[user_callbacks] = cb;
          callback_mem[user_callbacks++] = (char*)(rb->buffer_)->getMemory();
          callback = user_callbacks;
          fprintf(stderr, "------------ [T%x] %s:%d(%s) user_callbacks = %d\n", (unsigned int)pthread_self(), __FILE__, __LINE__, __func__, user_callbacks);
      }

      if(1 == callback)
      {
          r_.rbmi_.post_recv(transformCB,
              addr_,
              reinterpret_cast<iofwdutil::mm::BMIMemoryAlloc *>(compressed_mem[callback-1]),
              size,
              &(param_.mem_expected_size),
              BMI_EXT_ALLOC, tag_, 0);
      }
      else
      {
          /* Create a scope just for the scoped lock */
          {
              boost::mutex::scoped_lock l (mp_);
              total_slots = decompressed_size / param_.pipeline_size;
              remBytes = decompressed_size % param_.pipeline_size;
              if(remBytes > 0 &&
                 remBytes < param_.pipeline_size &&
                 iofwdutil::transform::TRANSFORM_STREAM_END == transform_->getTransformState())
              {
                total_slots++;
                partial_slot = true;
              }
              while(next_slot < std::min(user_callbacks, total_slots))
              {
                  offset = next_slot * param_.pipeline_size;
                  bytes = param_.pipeline_size;
                  if(true == partial_slot && next_slot == total_slots - 1)
                    bytes = remBytes;
                  memcpy(callback_mem[next_slot], decompressed_mem+offset, bytes);
                  userCB_[next_slot](recvStatus);
                  fprintf(stderr, "------------ [T%x]%s:%d(%s) next_slot = %d\n", (unsigned int)pthread_self(), __FILE__, __LINE__, __func__, next_slot);
                  fprintf(stderr, "------------ [T%x]%s:%d(%s) user_callbacks = %d\n", (unsigned int)pthread_self(), __FILE__, __LINE__, __func__, user_callbacks);
                  next_slot++;
              }
          }
      }
   }
}

void IOFWDWriteRequest::reply(const CBType & cb)
{
   simpleOptReply(cb, getReturnCode(), TSSTART <<
         encoder::EncVarArray(param_.file_sizes, param_.file_count));
}

//===========================================================================
   }
}
