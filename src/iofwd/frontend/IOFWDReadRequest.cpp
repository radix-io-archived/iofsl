#include "IOFWDReadRequest.hh"
#include "iofwdutil/bmi/BMIOp.hh"
#include "zoidfs/zoidfs-proto.h"

namespace iofwd
{
   namespace frontend
   {
//===========================================================================

IOFWDReadRequest::~IOFWDReadRequest ()
{
   if (mem_starts_) {
      for (uint32_t i = 0; i < mem_count_; i++)
         delete[] mem_starts_[i];
      delete[] mem_starts_;
   }
   if (mem_sizes_)
      delete[] mem_sizes_;
   if (file_starts_)
      delete[] file_starts_;
   if (file_sizes_)
      delete[] file_sizes_;
}

const IOFWDReadRequest::ReqParam & IOFWDReadRequest::decodeParam ()
{
   process (req_reader_, handle_);

   process (req_reader_, mem_count_);
   mem_starts_ = new char*[mem_count_];
   for (uint32_t i = 0; i < mem_count_; i++)
      mem_starts_[i] = new char[zoidfs::ZOIDFS_BUFFER_MAX];
   mem_sizes_ = new uint32_t[mem_count_];
   process (req_reader_, iofwdutil::xdr::XDRVarArray(mem_sizes_, mem_count_));

   process (req_reader_, file_count_);
   file_starts_ = new uint64_t[file_count_];
   process (req_reader_, iofwdutil::xdr::XDRVarArray(file_starts_, file_count_));
   file_sizes_ = new uint64_t[file_count_];
   process (req_reader_, iofwdutil::xdr::XDRVarArray(file_sizes_, file_count_));

   process (req_reader_, pipeline_size_);

   param_.handle = &handle_;
   param_.mem_count = mem_count_;
   param_.mem_starts = mem_starts_;
   param_.mem_sizes = mem_sizes_;
   param_.file_count = file_count_;
   param_.file_starts = file_starts_;
   param_.file_sizes = file_sizes_;
   param_.pipeline_size = pipeline_size_;
   return param_;
}

iofwdutil::completion::CompletionID * IOFWDReadRequest::sendBuffers ()
{
   size_t total_size = 0;
   for (size_t i = 0; i < mem_count_; i++)
      total_size += mem_sizes_[i];
   iofwdutil::completion::BMICompletionID * id = new iofwdutil::completion::BMICompletionID ();
   bmires_.postSendList (id, addr_, (const void**)mem_starts_, (const bmi_size_t*)mem_sizes_,
                         mem_count_, total_size, BMI_PRE_ALLOC, tag_, 0);
   return id;
}

iofwdutil::completion::CompletionID * IOFWDReadRequest::sendPipelineBuffer(char *buf, size_t size)
{
   iofwdutil::completion::BMICompletionID * id = new iofwdutil::completion::BMICompletionID ();
   bmires_.postSend (id, addr_, buf, size, BMI_PRE_ALLOC, tag_, 0);
   return id;
}


iofwdutil::completion::CompletionID * IOFWDReadRequest::reply()
{
   return simpleReply (TSSTART << (int32_t) getReturnCode ()
             << iofwdutil::xdr::XDRVarArray(file_sizes_, file_count_));
}

//===========================================================================
   }
}
