#ifndef IOFWD_REQUESTTASK_HH
#define IOFWD_REQUESTTASK_HH

#include <boost/function.hpp>
#include "iofwdutil/workqueue/WorkItem.hh"

namespace iofwd
{
//===========================================================================

class Request; 

/**
 * This class encodes the set of actions that need to be done to complete a
 * request from a client.
 */
class RequestTask : public iofwdutil::workqueue::WorkItem
{
public:

   /// Return codes for the run method. 
   enum { 
      STATUS_DONE = 0, // Request is done, can be destroyed
      STATUS_WAITING,  // Request waiting, will be manually rescheduled
      STATUS_RERUN     // Request can rerun immediately
   };  


   int getStatus () const
   { return status_; } 

   void setStatus (int status)
   { status_ = status; } 

   /// Return the request associated with this task
   /// The task owns the request and will destroy it
   Request * getRequest ()
   { return request_; } 

   /// Set the request associated with this task
   void setRequest (Request * req)
   { request_ = req; }

   // Fast requests can possibly take a shortcut and be serviced in the main
   // receiving thread; Note that a request that needs significant time to
   // determine if it is fast or not cannot be fast.
   virtual bool isFast () const
   { return false; }


   /// Called when the task gets the CPU
   virtual void run () = 0; 

   RequestTask (boost::function<void (RequestTask *)> & resched)
      : status_ (0), request_(0), reschedule_(resched)
   {
   }

   virtual ~RequestTask (); 
   
   
   /// Called if the task can be rescheduled for work
   void reschedule ()
   { reschedule_ (this); } 

protected:
   /// Called by the workqueue if we get the CPU
   virtual void doWork (); 


protected:
   int status_; 

   Request * request_; 

   boost::function<void (RequestTask *)> reschedule_; 
}; 

//===========================================================================
}

#endif
