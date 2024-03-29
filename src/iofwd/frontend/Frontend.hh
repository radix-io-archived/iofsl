#ifndef IOFWD_FRONTEND_HH
#define IOFWD_FRONTEND_HH

#include "iofwd/RequestHandler.hh"
#include "iofwdutil/ConfigFile.hh"

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


      void setConfig (const iofwdutil::ConfigFile & c);

      /// Called just after frontend instance is created
      /// If initialization fails, an exception should be thrown.
      virtual void init () = 0;

      /// Called when the frontend can start generating requests
      virtual void run () = 0;

      /// Called when shutting down. Should shut down internal threads
      /// and block until all threads are done.
      virtual void destroy () = 0; 

      virtual ~Frontend ();

   protected:
      RequestHandler * handler_; 

      iofwdutil::ConfigFile config_;
};

//===========================================================================
   }
}

#endif
