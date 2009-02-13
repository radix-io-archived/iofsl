#include "IOFWDRequest.hh"

namespace iofwd
{
   namespace frontend
   {
//===========================================================================

IOFWDRequest::IOFWDRequest (const BMI_unexpected_info & info)
   : raw_request_ (info), addr_ (raw_request_.getAddr()),
   tag_(raw_request_.getTag())
{
}

IOFWDRequest::~IOFWDRequest ()
{
}

//===========================================================================
   }
}
