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

   if(NULL != compressed_mem_)
   {
      delete []compressed_mem_[0];
      compressed_mem_[0] = NULL;
   }

   delete []compressed_mem_;
   compressed_mem_ = NULL;

   delete []decompressed_mem_;
   decompressed_mem_ = NULL;

   delete []callback_mem_;
   callback_mem_ = NULL;

   delete []userCB_;
   userCB_ = NULL;

   delete []mem_expected_size_;
   mem_expected_size_ = NULL;

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

   h.hafree(compressed_mem_[0]);
   compressed_mem_[0] = NULL;

   h.hafree(compressed_mem_);
   compressed_mem_ = NULL;

   h.hafree(decompressed_mem_);
   decompressed_mem_ = NULL;

   h.hafree(callback_mem_);
   callback_mem_ = NULL;

   h.hafree(userCB_);
   userCB_ = NULL;

   h.hafree(mem_expected_size_);
   mem_expected_size_ = NULL;

#endif
   if(param_.op_hint)
      zoidfs::util::ZoidFSHintDestroy(&(param_.op_hint));
}

IOFWDWriteRequest::ReqParam & IOFWDWriteRequest::decodeParam ()
{
   // init the handle
   process (req_reader_, handle_);

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

   // init the mem count and sizes
   param_.mem_count = param_.file_count;
#ifndef USE_TASK_HA
   param_.mem_sizes = new size_t[param_.file_count];
#else
   param_.mem_sizes = (h.hamalloc<size_t>(param_.file_count));
#endif

   for(size_t ii = 0; ii < param_.file_count ; ii++)
   {
      param_.mem_sizes[ii] = param_.file_sizes[ii];
   }

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

   char *enable_transform = zoidfs::util::ZoidFSHintGet(&(param_.op_hint), ZOIDFS_TRANSFORM);
   if(NULL != enable_transform)
   {
        char *token = NULL, *saveptr = NULL;

        token = strtok_r(enable_transform, ":", &saveptr);

        if(NULL != token)
        {
           transform_.reset
              (iofwdutil::transform::GenericTransformFactory::construct(token)());
	   op_hint_compress_enabled_ = true;
	   if (0 == param_.pipeline_size)
	   {
	      char *compressed_size_str =
		zoidfs::util::ZoidFSHintGet(&(param_.op_hint), ZOIDFS_COMPRESSED_SIZE);
	      ASSERT(compressed_size_str != NULL);
	      compressed_size_ = (size_t)atol(compressed_size_str);
	      ASSERT(compressed_size_ > 0);
	   }
        }
        else
        {
            op_hint_compress_enabled_ = false;
        }
   }

   char *enable_header_stuffing = zoidfs::util::ZoidFSHintGet(&(param_.op_hint), ZOIDFS_HEADER_STUFFING);
   if(enable_header_stuffing)
   {
	size_t uncompressed_bytes = 0;
        char *token = NULL, *saveptr = NULL;

        token = strtok_r(enable_header_stuffing, ":", &saveptr);

        if(NULL != token)
            uncompressed_bytes = (size_t)atol(token);

        if(0 != uncompressed_bytes)
          op_hint_headstuff_enabled_ = true;
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
	    token = strtok_r(NULL, ":", &saveptr);
	    hash_value_ = token;
	    op_hint_crc_enabled_ = true;
        }
   }

   // init the rest of the write request params to 0
   param_.mem_total_size = 0;
   param_.mem_expected_size = 0;

   // init the handle
   param_.handle = &handle_;

   // get the max buffer size from BMI
   r_.rbmi_.get_info(addr_, BMI_CHECK_MAXSIZE, static_cast<void *>(&param_.max_buffer_size));

   if(0 != param_.pipeline_size &&
      (true == op_hint_compress_enabled_ || true == op_hint_headstuff_enabled_))
   {
     param_.mem_total_size = 0;
     for(size_t ii = 0; ii < param_.mem_count; ii++)
       param_.mem_total_size += param_.mem_sizes[ii];

     pipeline_ops_ = param_.mem_total_size / param_.pipeline_size;

     if(0 != param_.mem_total_size % param_.pipeline_size)
       pipeline_ops_++;

#ifndef USE_TASK_HA
     compressed_mem_ = new char* [pipeline_ops_];
#else
     compressed_mem_ = (h.hamalloc<char*>(pipeline_ops_));
#endif

#ifndef USE_TASK_HA
     char *compmem = new char [param_.mem_total_size];
#else
     char *compmem = (h.hamalloc<char> (param_.mem_total_size));
#endif

     size_t offset = 0;

     for(size_t ii = 0; ii < pipeline_ops_; ii++)
     {
       compressed_mem_[ii] = compmem + offset;
       offset += param_.pipeline_size;
     }

     compressed_size_ = 0;

#ifndef USE_TASK_HA
     decompressed_mem_ = new char [param_.mem_total_size];
#else
     decompressed_mem_ = (h.hamalloc<char> (param_.mem_total_size));
#endif

     decompressed_size_ = 0;

#ifndef USE_TASK_HA
     callback_mem_ = new char* [pipeline_ops_];
#else
     callback_mem_ = (h.hamalloc<char*> (pipeline_ops_));
#endif

     for(size_t ii = 0; ii < pipeline_ops_; ii++)
	callback_mem_[ii] = NULL;

#ifndef USE_TASK_HA
     userCB_ = new CBType [pipeline_ops_];
#else
     userCB_ = (h.hamalloc<CBType> (pipeline_ops_));
#endif

     for(size_t ii = 0; ii < pipeline_ops_; ii++)
          userCB_[ii] = NULL;

     next_slot_ = 0;
     user_callbacks_ = 0;

#ifndef USE_TASK_HA
     mem_expected_size_ = new bmi_size_t [pipeline_ops_];
#else
     mem_expected_size_ = (h.hamalloc<bmi_size_t> (pipeline_ops_));
#endif

     for(size_t ii = 0; ii < pipeline_ops_; ii++)
	mem_expected_size_[ii] = 0;

     int size_of_packet = raw_request_.size();
     void *ptr_to_header = raw_request_.get();
     int size_of_header = req_reader_.getPos();
     int outState = 0;
     size_t outBytes = 0;
     char *src = NULL, *dst = NULL;

     size_of_stuffed_data_ = size_of_packet - size_of_header;
     src = (char*)ptr_to_header + size_of_header;
     dst = decompressed_mem_ + decompressed_size_;

     if(true == op_hint_headstuff_enabled_)
     {
       if(false == op_hint_compress_enabled_ || enable_pipeline)
       {
	 memcpy(dst, src, size_of_stuffed_data_);
	 decompressed_size_ += size_of_stuffed_data_;
       }
       else
       {
	  transform_->transform(src,
	    size_of_stuffed_data_,
	    dst,
	    param_.mem_total_size-decompressed_size_,
	    &outBytes,
	    &outState,
	    true);

	  ASSERT(iofwdutil::transform::CONSUME_OUTBUF != outState);
	  ASSERT(outBytes <= (param_.mem_total_size - decompressed_size_));

	  decompressed_size_ += outBytes;
       }
     }
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

	int size_of_packet = raw_request_.size();
	void *ptr_to_header = raw_request_.get();
	int size_of_header = req_reader_.getPos();

	size_of_stuffed_data_ = size_of_packet - size_of_header;

	if(true == op_hint_compress_enabled_ ||
	   true == op_hint_headstuff_enabled_)
	{
#ifndef USE_TASK_HA
	  compressed_mem_ = new char* [1];
#else
	  compressed_mem_ = (h.hamalloc<char*> (1));
#endif

	  if(true == op_hint_headstuff_enabled_)
	    compressed_size_ = param_.mem_total_size - size_of_stuffed_data_;
#ifndef USE_TASK_HA
	  compressed_mem_[0] = new char [compressed_size_];
#else
	  compressed_mem_[0] = (h.hamalloc<char> (compressed_size_));
#endif

#ifndef USE_TASK_HA
	  userCB_ = new CBType[1];
#else
	  userCB_ = (h.hamalloc<CBType> (1));
#endif
	  userCB_[0] = NULL;
	}

        if(true == op_hint_headstuff_enabled_)
        {
          unsigned int ii = 0;
          size_t bytes = 0, total_bytes = 0;
          char *position = NULL;
          int outState = 0;
          size_t outBytes = 0;

          position = (char*)ptr_to_header + size_of_header;

          if(false == op_hint_compress_enabled_)
          {
            for(ii = 0;
                ii < param_.mem_count && total_bytes < size_of_stuffed_data_;
                ii++, position += bytes, total_bytes += bytes)
            {
                bytes = std::min(param_.mem_sizes[ii], size_of_stuffed_data_-total_bytes);
                memcpy(param_.mem_starts[ii], position, bytes);
                if(true == op_hint_crc_enabled_)
		    hashFunc_->process (param_.mem_starts[ii], bytes);
            }
            mem_slot_ = ii - 1;
            mem_slot_bytes_ = bytes;
          }
          else
          {
            for(ii = 0;
                ii < param_.mem_count;
                ii++)
            {
                transform_->transform(position,
                  size_of_stuffed_data_,
                  param_.mem_starts[ii],
                  param_.mem_sizes[ii],
                  &outBytes,
                  &outState,
                  true);

                total_bytes += outBytes;
                mem_slot_bytes_ = outBytes;

		if(true == op_hint_crc_enabled_)
		  hashFunc_->process (param_.mem_starts[ii], outBytes);

                if(iofwdutil::transform::CONSUME_OUTBUF == outState)
                {
                    mem_slot_ = ii + 1;
		    mem_slot_bytes_ = 0;
                    continue;
                }
                else if(iofwdutil::transform::SUPPLY_INBUF == outState)
                {
                    mem_slot_ = ii;
                    break;
                }
                if(total_bytes == param_.mem_total_size)
                {
                    mem_slot_ = ii;
                    break;
                }
            }
            size_of_stuffed_data_ = total_bytes;
          }

	  decompressed_size_ = size_of_stuffed_data_;
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

   ASSERT(1 == user_callbacks_);
   ASSERT(size_of_stuffed_data_ < param_.mem_total_size);

   if(true == op_hint_compress_enabled_)
   {
      for(i = mem_slot_; i < param_.mem_count; i++)
      {
	  if(i == mem_slot_)
	  {
	      transform_->transform(compressed_mem_[0],
		param_.mem_expected_size,
		param_.mem_starts[i]+mem_slot_bytes_,
		param_.mem_sizes[i]-mem_slot_bytes_,
		&outBytes,
		&outState,
		false);

	      decompressed_size_ += outBytes;

	      if(true == op_hint_crc_enabled_)
		hashFunc_->process (param_.mem_starts[i]+mem_slot_bytes_, outBytes);
	  }
	  else
	  {
	      transform_->transform(compressed_mem_[0],
		param_.mem_expected_size,
		param_.mem_starts[i],
		param_.mem_sizes[i],
		&outBytes,
		&outState,
		false);

	      decompressed_size_ += outBytes;

	      if(true == op_hint_crc_enabled_)
		hashFunc_->process (param_.mem_starts[i], outBytes);
	  }

	  if(iofwdutil::transform::CONSUME_OUTBUF == outState)
	  {
	      continue;
	  }
	  if(decompressed_size_ == param_.mem_total_size)
	  {
	      if(true == op_hint_crc_enabled_)
	      {
		  const size_t bufsize = hashFunc_->getHashSize();
		  boost::scoped_array<char> result (new char[bufsize]);

		  if (0 != strcasecmp (result.get(), hash_value_))
		  fprintf(stderr, "Incompatible hash values\n"
				  "hash value received over wire = %s\n"
				  "hash value calculated = %s\n"
				  , hash_value_, result.get());
	      }
	      userCB_[0](recvStatus);
	      break;
	  }
	  ASSERT(iofwdutil::transform::SUPPLY_INBUF != outState);
      }
   }
   else
   {
      size_t bytes = param_.mem_sizes[mem_slot_] - mem_slot_bytes_;

      memcpy(param_.mem_starts[mem_slot_]+mem_slot_bytes_, compressed_mem_[0],
	      bytes);

      if(true == op_hint_crc_enabled_)
	hashFunc_->process (param_.mem_starts[mem_slot_]+mem_slot_bytes_, bytes);

      for(i = mem_slot_+1; i < param_.mem_count; i++)
      {
	  memcpy(param_.mem_starts[i], compressed_mem_[0]+bytes, param_.mem_sizes[i]);

	  if(true == op_hint_crc_enabled_)
	    hashFunc_->process (&param_.mem_starts[i], param_.mem_sizes[i]);

	  bytes += param_.mem_sizes[i];
      }

      userCB_[0](recvStatus);
   }
}

void IOFWDWriteRequest::recvBuffers(const CBType & cb, RetrievedBuffer * rb)
{
    int recvStatus = 0;

    param_.mem_expected_size = 0;

    if(false == op_hint_compress_enabled_ &&
       false == op_hint_headstuff_enabled_)
    {
#if SIZEOF_SIZE_T == SIZEOF_INT64_T
      r_.rbmi_.post_recv_list(cb,
	  addr_,
	  reinterpret_cast<void*const*>(param_.mem_starts),
	  reinterpret_cast<const bmi_size_t *>(param_.mem_sizes),
	  param_.mem_count,
	  param_.mem_total_size,
	  &(param_.mem_expected_size),
	  dynamic_cast<iofwdutil::mm::BMIMemoryAlloc *>(rb->buffer_)->bmiType(),
	  tag_, 0);
#else
      r_.rbmi_.post_recv_list(cb,
	  addr_,
	  reinterpret_cast<void*const*>(param_.mem_starts),
	  reinterpret_cast<const bmi_size_t*>(param_.bmi_mem_sizes),
	  param_.mem_count,
	  param_.mem_total_size,
	  &(param_.mem_expected_size),
	  dynamic_cast<iofwdutil::mm::BMIMemoryAlloc *>(rb->buffer_)->bmiType(),
	  tag_, 0);
#endif
    }
    else
    {
      CBType transformCB = boost::bind(&IOFWDWriteRequest::recvComplete,
            boost::ref(*this), _1);

      userCB_[user_callbacks_++] = cb;

      ASSERT(1 == user_callbacks_);

      //STATIC_ASSERT(sizeof(bmi_size_t) == sizeof(size_t));

#if SIZEOF_SIZE_T == SIZEOF_INT64_T
      if(size_of_stuffed_data_ < param_.mem_total_size)
      {
          r_.rbmi_.post_recv_list(transformCB,
              addr_,
              reinterpret_cast<void*const*>(compressed_mem_),
              reinterpret_cast<const bmi_size_t*>(&compressed_size_),
              1,
              param_.mem_total_size-size_of_stuffed_data_,
              &(param_.mem_expected_size),
              BMI_EXT_ALLOC,
              tag_, 0);
      }
      else
      {
          userCB_[0](recvStatus);
      }
#else
      if(size_of_stuffed_data_ < param_.mem_total_size)
      {
          r_.rbmi_.post_recv_list(transformCB,
              addr_,
              reinterpret_cast<void*const*>(compressed_mem_),
              reinterpret_cast<const bmi_size_t*>(&compressed_size_),
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
   size_t remBytes = 0;
   size_t offset = 0;
   size_t bytes = 0;
   size_t pipelines_posted = 0;
   bool partial_slot = false;

   ASSERT(decompressed_size_ < param_.mem_total_size);

   if(true == op_hint_compress_enabled_)
   {
      transform_->transform(compressed_mem_[my_slot],
	    mem_expected_size_[my_slot],
	    decompressed_mem_+decompressed_size_,
	    param_.mem_total_size-decompressed_size_,
	    &outBytes,
	    &outState,
	    true);

      ASSERT(iofwdutil::transform::CONSUME_OUTBUF != outState);
      ASSERT(outBytes <= (param_.mem_total_size - decompressed_size_));

      if(true == op_hint_crc_enabled_)
          hashFunc_->process (decompressed_mem_+decompressed_size_, outBytes);

      decompressed_size_ += outBytes;

      pipelines_posted = decompressed_size_ / param_.pipeline_size;
      remBytes = decompressed_size_ % param_.pipeline_size;

      if(remBytes > 0 && decompressed_size_ == param_.mem_total_size)
      {
	  pipelines_posted++;
	  partial_slot = true;
      }

      if(pipelines_posted > 0)
      {
	  boost::mutex::scoped_lock l (mp_);

	  offset = next_slot_ * param_.pipeline_size;

	  while(next_slot_ < std::min(user_callbacks_, pipelines_posted))
	  {
	      bytes = param_.pipeline_size;
	      if(true == partial_slot && next_slot_ == pipelines_posted - 1)
		bytes = remBytes;
	      memcpy(callback_mem_[next_slot_], decompressed_mem_+offset, bytes);
	      userCB_[next_slot_](recvStatus);
	      next_slot_++;
	      offset += bytes;
	  }
      }

      if(iofwdutil::transform::SUPPLY_INBUF == outState)
      {
	  CBType transformCB = boost::bind(&IOFWDWriteRequest::recvPipelineComplete,
	      boost::ref(*this), _1, my_slot+1);

	  r_.rbmi_.post_recv(transformCB,
	      addr_,
	      compressed_mem_[my_slot+1],
	      param_.pipeline_size,
	      &(param_.mem_expected_size), 
	      BMI_EXT_ALLOC, tag_, 0);
      }

      if(decompressed_size_ == param_.mem_total_size)
      {
	  if(true == op_hint_crc_enabled_)
	  {
	      const size_t bufsize = hashFunc_->getHashSize();
	      boost::scoped_array<char> result (new char[bufsize]);

	      if (0 != strcasecmp (result.get(), hash_value_))
		fprintf(stderr, "Incompatible hash values\n"
		"hash value received over wire = %s\n"
		"hash value calculated = %s\n"
		, hash_value_, result.get());
	  }
      }
   }
   else
   {
     {
	memcpy(decompressed_mem_+decompressed_size_, compressed_mem_[my_slot],
	    mem_expected_size_[my_slot]);

	if(true == op_hint_crc_enabled_)
	  hashFunc_->process (decompressed_mem_+decompressed_size_,
	      mem_expected_size_[my_slot]);

	decompressed_size_ += mem_expected_size_[my_slot];

	pipelines_posted = decompressed_size_ / param_.pipeline_size;
	remBytes = decompressed_size_ % param_.pipeline_size;

	if(remBytes > 0 && decompressed_size_ == param_.mem_total_size)
	{
	    pipelines_posted++;
	    partial_slot = true;
	}

	{
	  boost::mutex::scoped_lock l (mp_);

	  offset = next_slot_ * param_.pipeline_size;

	  while(next_slot_ < std::min(user_callbacks_, pipelines_posted))
	  {
	      bytes = param_.pipeline_size;
	      if(true == partial_slot && next_slot_ == pipelines_posted - 1)
		bytes = remBytes;
	      memcpy(callback_mem_[next_slot_], decompressed_mem_+offset, bytes);
	      userCB_[next_slot_](recvStatus);
	      next_slot_++;
	      offset += bytes;
	  }
	}
     }

     if(decompressed_size_ < param_.mem_total_size)
     {
	CBType transformCB = boost::bind(&IOFWDWriteRequest::recvPipelineComplete,
	    boost::ref(*this), _1, my_slot+1);

	r_.rbmi_.post_recv(transformCB,
	    addr_,
	    compressed_mem_[my_slot+1],
	    std::min(param_.mem_total_size-decompressed_size_, param_.pipeline_size),
	    &(mem_expected_size_[my_slot+1]), 
	    BMI_EXT_ALLOC, tag_, 0);
     }
     else
     {
   	  if(true == op_hint_crc_enabled_)
   	  {
	      const size_t bufsize = hashFunc_->getHashSize();
	      boost::scoped_array<char> result (new char[bufsize]);

	      if (0 != strcasecmp (result.get(), hash_value_))
		fprintf(stderr, "Incompatible hash values\n"
				"hash value received over wire = %s\n"
				"hash value calculated = %s\n"
				, hash_value_, result.get());
   	  }
     }
   }
}

void IOFWDWriteRequest::recvPipelineBufferCB(iofwdevent::CBType cb, RetrievedBuffer * rb, size_t size)
{
   size_t total_slots = 0;
   int recvStatus = 0;
   size_t offset = 0;
   size_t bytes = 0;
   size_t remBytes = 0;
   bool partial_slot = false;

   if(false == op_hint_compress_enabled_ &&
      false == op_hint_headstuff_enabled_)
   {
      r_.rbmi_.post_recv(cb,
	  addr_,
	  dynamic_cast<iofwdutil::mm::BMIMemoryAlloc *>(rb->buffer_)->getMemory(),
	  size,
	  &(param_.mem_expected_size), 
          dynamic_cast<iofwdutil::mm::BMIMemoryAlloc *>(rb->buffer_)->bmiType(),
	  tag_, 0);
   }
   else
   {
      CBType transformCB = boost::bind(&IOFWDWriteRequest::recvPipelineComplete,
	  boost::ref(*this), _1, user_callbacks_);
      int callback = 0;

      {
          boost::mutex::scoped_lock l (mp_);
          userCB_[user_callbacks_] = cb;
          callback = user_callbacks_;
          callback_mem_[user_callbacks_++] = (char*)(rb->buffer_)->getMemory();
      }

      if(0 == callback)
      {
	if(decompressed_size_ < param_.mem_total_size)
	{
          r_.rbmi_.post_recv(transformCB,
              addr_,
              reinterpret_cast<iofwdutil::mm::BMIMemoryAlloc *>(compressed_mem_[callback]),
              size,
              &mem_expected_size_[callback],
              BMI_EXT_ALLOC, tag_, 0);
	}
	else
	{
          {
              total_slots = decompressed_size_ / param_.pipeline_size;
              remBytes = decompressed_size_ % param_.pipeline_size;

              if(remBytes > 0 &&
                 decompressed_size_ == param_.mem_total_size)
              {
                total_slots++;
                partial_slot = true;
              }

              boost::mutex::scoped_lock l (mp_);

	      offset = next_slot_ * param_.pipeline_size;

              while(next_slot_ < std::min(user_callbacks_, total_slots))
              {
                  bytes = param_.pipeline_size;
                  if(true == partial_slot && next_slot_ == total_slots - 1)
                    bytes = remBytes;
                  memcpy(callback_mem_[next_slot_], decompressed_mem_+offset, bytes);
                  userCB_[next_slot_](recvStatus);
                  next_slot_++;
		  offset += bytes;
              }
          }
	}
      }
      else
      {
          {
              total_slots = decompressed_size_ / param_.pipeline_size;
              remBytes = decompressed_size_ % param_.pipeline_size;

              if(remBytes > 0 &&
                 decompressed_size_ == param_.mem_total_size)
              {
                total_slots++;
                partial_slot = true;
              }

              boost::mutex::scoped_lock l (mp_);

	      offset = next_slot_ * param_.pipeline_size;

              while(next_slot_ < std::min(user_callbacks_, total_slots))
              {
                  bytes = param_.pipeline_size;
                  if(true == partial_slot && next_slot_ == total_slots - 1)
                    bytes = remBytes;
                  memcpy(callback_mem_[next_slot_], decompressed_mem_+offset, bytes);
                  userCB_[next_slot_](recvStatus);
                  next_slot_++;
		  offset += bytes;
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
