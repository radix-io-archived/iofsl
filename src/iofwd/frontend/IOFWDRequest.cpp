#include "IOFWDRequest.hh"
#include "iofwdutil/bmi/BMIOp.hh"

namespace iofwd
{
   namespace frontend
   {
//===========================================================================

IOFWDRequest::IOFWDRequest (const BMI_unexpected_info & info,
      IOFWDResources & res)
   :
   r_ (res),
   bmi_ (*r_.bmictx_), raw_request_ (info), addr_ (raw_request_.getAddr()),
   tag_(raw_request_.getTag()),
   req_reader_(raw_request_.get(), raw_request_.size()),
   buffer_send_ (addr_, iofwdutil::bmi::BMI::ALLOC_SEND)
{
   // opid may not be used
   int32_t opid;
   process(req_reader_, opid);
}

IOFWDRequest::~IOFWDRequest ()
{
  raw_request_.free();
  buffer_send_.resize(0);

  // The unexpected/send buffer is already freeed.
  // Then, decrement the reference count in the BMI layer, associate with
  // the address.
  BMI_set_info(addr_, BMI_DEC_ADDR_REF, NULL);
}

void IOFWDRequest::beginReply (size_t maxsize)
{
   reply_writer_.reset (buffer_send_.get(maxsize), maxsize);
}

void IOFWDRequest::sendReply (const iofwdevent::CBType & cb)
{
   ZLOG_DEBUG_EXTREME (r_.log_, iofwdutil::format("Sending reply of %lu bytes (max bufsize = %lu bytes)")
             % reply_writer_.size() % reply_writer_.getMaxSize());

   // beginReply allocated BMI mem so we can us BMI_PRE_ALLOC
   // The server responds using the same tag as the incoming request.
   r_.rbmi_.post_send (cb, addr_, reply_writer_.getBuf(),
         reply_writer_.size (), BMI_PRE_ALLOC, tag_, 0);
}

/**
 * \brief Free memory associated with unexpected message.
 *
 * Currently this is not used. In principle, the derived 
 * IOFWDRequests could call this function to free memory as soon as possible
 * (i.e. as soon as the request parameters are decoded).
 *
 * If it is not called, memory will be freed by the BMIUnexpectedBuffer
 * when the request is destructed.
 */
void IOFWDRequest::freeRawRequest ()
{
   raw_request_.free ();
}

//===========================================================================
   }
}
