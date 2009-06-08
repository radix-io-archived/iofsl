#ifndef IOFWD_COMMITREQUEST_HH
#define IOFWD_COMMITREQUEST_HH

#include "Request.hh"
#include "iofwdutil/completion/CompletionID.hh"

namespace iofwd
{

   class CommitRequest : public Request
   {
      public:
         CommitRequest (int opid) : Request (opid)
         {
         }; 

         virtual iofwdutil::completion::CompletionID * reply () = 0; 

         virtual ~CommitRequest (); 
   };

}

#endif
