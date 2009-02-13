#ifndef IOFWD_FRONTEND_IOFWDLOOKUPREQUEST_HH
#define IOFWD_FRONTEND_IOFWDLOOKUPREQUEST_HH

#include "IOFWDRequest.hh"
#include "iofwd/LookupRequest.hh"

namespace iofwd
{
   namespace frontend
   {

class IOFWDLookupRequest 
   : public IOFWDRequest, 
     public LookupRequest
{
public:
   IOFWDLookupRequest (int opid, const BMI_unexpected_info & info)
      : IOFWDRequest (info), LookupRequest (opid)
   {
   }
   
   virtual void reply (const zoidfs_handle_t * handle); 

   virtual ~IOFWDLookupRequest (); 
}; 

   }
}

#endif
