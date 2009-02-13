#ifndef IOFWD_FRONTEND_HH
#define IOFWD_FRONTEND_HH

#include "iofwd/RequestHandler.hh"

namespace iofwd
{
   namespace frontend
   {
//===========================================================================


/**
 * Currently, we assume the frontend uses its own threads 
 * (i.e. we do not need to poll it and the frontend calls our handler
 * when something comes in)
 */
class Frontend 
{
   public:

      void setHandler (RequestHandler * handler); 

      /// Called when the frontend can start generating requests
      /// If initialization fails, an exception should be thrown.
      virtual void init () = 0; 

      /// Called when shutting down. Should shut down internal threads
      /// and block until all threads are done.
      virtual void destroy () = 0; 

      virtual ~Frontend ();

   protected:
      RequestHandler * handler_; 
};

//===========================================================================
   }
}

#endif
