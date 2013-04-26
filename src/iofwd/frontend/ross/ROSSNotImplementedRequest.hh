#ifndef ROSS_FRONTENT_ROSSNOTIMPLEMENTEDREQUEST_HH
#define ROSS_FRONTENT_ROSSNOTIMPLEMENTEDREQUEST_HH

#include "ROSSRequest.hh"
#include "iofwd/NotImplementedRequest.hh"

namespace iofwd
{
   namespace rossfrontend
   {

class ROSSNotImplementedRequest : public NotImplementedRequest,
                                   public ROSSRequest
{
public:
   ROSSNotImplementedRequest(int opid)
      : NotImplementedRequest(opid), ROSSRequest()
   {
   }


   virtual void reply(const CBType & cb);

   virtual ~ROSSNotImplementedRequest();


};

   }
}

#endif
