#include "Request.hh"

using namespace iofwdutil::bmi;

namespace iofwd
{
//===========================================================================

Request::Request (int opid, BMIAddr addr, BMITag tag)
   : opid_(opid), addr_(addr), tag_(tag)
{
}

Request::~Request ()
{
   // maybe log here 
}

//===========================================================================
}
