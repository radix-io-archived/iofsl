#ifndef IOFWD_DEFREQUESTHANDLER_HH
#define IOFWD_DEFREQUESTHANDLER_HH

#include "iofwdutil/IOFWDLog.hh"
#include "RequestHandler.hh"

namespace iofwd
{

/**
 * This class accepts requests from the frontend and uses a workqueue to run
 * the requests until the request returns false (indicating it is done)
 */
class DefRequestHandler : public RequestHandler
{
public:

   /// Accept requests and put them on the workqueue
   virtual void handleRequest (int count, Request ** reqs); 

   DefRequestHandler (); 

   virtual ~DefRequestHandler (); 

protected:
   iofwdutil::IOFWDLogSource & log_; 

};

}

#endif
