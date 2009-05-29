#ifndef IOFWD_READREQUEST_HH
#define IOFWD_READREQUEST_HH

#include "Request.hh"
#include <cstring>

namespace iofwd
{
   
   class ReadRequest : public Request 
   {
      public:
       ReadRequest (int opid)
          : Request (opid)
       {
       }

       virtual void returnData (const void * buf[], const size_t size[],
             int count) = 0; 

       virtual ~ReadRequest (); 

   }; 

}

#endif
