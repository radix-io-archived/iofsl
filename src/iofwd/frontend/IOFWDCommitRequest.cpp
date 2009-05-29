#include "IOFWDCommitRequest.hh"

namespace iofwd
{
   namespace frontend
   {
//===========================================================================

      IOFWDCommitRequest::~IOFWDCommitRequest ()
      {
      }

      void IOFWDCommitRequest::reply ()
      {
         simpleReply (TSSTART << (int32_t) getReturnCode ()); 
      }

//===========================================================================
   }
}
