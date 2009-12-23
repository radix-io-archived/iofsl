#ifndef IOFWD_FRONTENT_IOFWDNOTIMPLEMENTEDREQUEST_HH
#define IOFWD_FRONTENT_IOFWDNOTIMPLEMENTEDREQUEST_HH

#include "IOFWDRequest.hh"
#include "iofwd/NotImplementedRequest.hh"

namespace iofwd
{
   namespace frontend
   {

class IOFWDNotImplementedRequest : public NotImplementedRequest,
                                   public IOFWDRequest
{
public:
   IOFWDNotImplementedRequest (int opid, const BMI_unexpected_info & info,
         IOFWDResources & res)
      : NotImplementedRequest(opid), IOFWDRequest(info,res)
   {
   }


   virtual void reply ();

   virtual ~IOFWDNotImplementedRequest ();


};

   }
}

#endif
