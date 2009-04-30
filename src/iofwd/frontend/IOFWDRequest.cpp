#include "IOFWDRequest.hh"
#include "iofwdutil/bmi/BMIOp.hh"

namespace iofwd
{
   namespace frontend
   {
//===========================================================================

IOFWDRequest::IOFWDRequest (iofwdutil::bmi::BMIContext & bmi, const BMI_unexpected_info & info)
   : bmi_ (bmi), raw_request_ (info), addr_ (raw_request_.getAddr()),
   tag_(raw_request_.getTag()), 
   req_reader_(raw_request_.get(), raw_request_.size()),
   buffer_send_ (addr_, iofwdutil::bmi::BMI::ALLOC_SEND)
{
}

IOFWDRequest::~IOFWDRequest ()
{
}

void IOFWDRequest::beginReply (size_t maxsize)
{
   reply_writer_.reset (buffer_send_.get(maxsize), maxsize); 
}

iofwdutil::bmi::BMIOp IOFWDRequest::sendReply ()
{
   // beginReply allocated BMI mem
   return ll_sendReply (reply_writer_.getBuf(), reply_writer_.size(), 
         BMI_PRE_ALLOC); 
}

void IOFWDRequest::freeRawRequest ()
{
   raw_request_.free (); 
}

inline iofwdutil::bmi::BMIOp IOFWDRequest::ll_sendReply (const void * buf, size_t bufsize,
      bmi_buffer_type type)
{
   // Server replies with same tag
   return bmi_.postSend (addr_, buf, bufsize, type, tag_); 
}

//===========================================================================
   }
}
