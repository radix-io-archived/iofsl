#ifndef IOFWD_REQUESTHANDLER_HH
#define IOFWD_REQUESTHANDLER_HH

namespace iofwd
{
//===========================================================================

class Request; 

/**
 * Input channel for requests...
 */
class RequestHandler
{
public:

   /// Called when one or more requests are available
   /// Ownership is transferred (i.e. RequestHandler has to free the requests)
   virtual void handleRequest (int count, Request * reqs) =0; 

   virtual ~RequestHandler (); 
}; 

//===========================================================================
}

#endif
