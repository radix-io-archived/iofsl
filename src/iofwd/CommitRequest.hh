#ifndef IOFWD_COMMITREQUEST_HH
#define IOFWD_COMMITREQUEST_HH

#include "Request.hh"

namespace iofwd
{

   class CommitRequest : public Request
   {
      public:
         CommitRequest (int opid) : Request (opid)
         {
         }; 

         virtual void reply () = 0; 

         virtual ~CommitRequest (); 
   };

}

#endif
