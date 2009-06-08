#include "IOFWDCommitRequest.hh"

namespace iofwd
{
   namespace frontend
   {
//===========================================================================

      IOFWDCommitRequest::~IOFWDCommitRequest ()
      {
      }

      iofwdutil::completion::CompletionID * IOFWDCommitRequest::reply ()
      {
         return simpleReply (TSSTART << (int32_t) getReturnCode ()); 
      }

//===========================================================================
   }
}
