#ifndef IOFWD_READREQUEST_HH
#define IOFWD_READREQUEST_HH

#include <cstring>
#include "Request.hh"
#include "iofwdutil/completion/CompletionID.hh"

namespace iofwd
{
   
   class ReadRequest : public Request 
   {
      public:
       ReadRequest (int opid)
          : Request (opid)
       {
       }

       virtual iofwdutil::completion::CompletionID * returnData (const void * buf[], const size_t size[],
             int count) = 0; 

       virtual ~ReadRequest (); 

   }; 

}

#endif
