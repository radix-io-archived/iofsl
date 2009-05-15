#ifndef IOFWD_REQUEST_HH
#define IOFWD_REQUEST_HH

#include <boost/function.hpp>

namespace iofwd
{
//===========================================================================

/**
 * This class represents a request from a client. It contains the necessary
 * information to reply to the client.
 *
 *
 */
class Request 
{
public:

   /// Return codes for the run method. 
   typedef enum { 
      STATUS_DONE = 0, // Request is done, can be destroyed
      STATUS_WAITING,  // Request waiting, will be manually rescheduled
      STATUS_RERUN     // Request can rerun immediately
   } reqstatus;  

   Request (int opid); 

   // Return the ZoidFS operation associated with this request 
   int getOpID () const
   {
      return opid_; 
   }; 

   virtual void setStatus (int status)
   {
      status_ = status; 
   }

   // Called in the worker thread when the request gets some cputime
   // Needs to return false if all work is done and the request can be
   // destroyed
   virtual reqstatus run () = 0; 

   // Fast requests can possibly take a shortcut and be serviced in the main
   // receiving thread; Note that a request that needs significant time to
   // determine if it is fast or not cannot be fast.
   virtual bool isFast () 
   { return false; }


   const char * opid2Name (int opid) const; 

   const char * opid2Name () const
   { return opid2Name (opid_); }

   virtual ~Request (); 

   /// Can be called to resume the request
   void resume ()
   {
      reschedule_ (); 
   }

   /// Sets the function to reschedule the requests
   void setResume (const boost::function< void () > & func)
   {
      reschedule_ = func; 
   }
 
protected:
   // Operation
   int opid_; 

   // Return code
   int status_; 

   // Callback to reschedule request
   boost::function<void ()> reschedule_; 
}; 

//===========================================================================
}

#endif
