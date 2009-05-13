#include "DefRequestHandler.hh"
#include "Request.hh"

using namespace iofwdutil; 

namespace iofwd
{
//===========================================================================

DefRequestHandler::DefRequestHandler ()
   : log_ (IOFWDLog::getSource ("defreqhandler"))
{
}

DefRequestHandler::~DefRequestHandler ()
{
}

void DefRequestHandler::handleRequest (int count, Request ** reqs)
{
   ZLOG_DEBUG(log_, str(format("handleRequest: %u requests") % count)); 
   for (int i=0; i<count; ++i)
      delete (reqs[i]); 
}

//===========================================================================
}
