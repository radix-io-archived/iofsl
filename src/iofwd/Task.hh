#ifndef IOFWD_TASK_HH
#define IOFWD_TASK_HH

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
class Task : public iofwdutil::workqueue::WorkItem
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

   void setTaskAllocType(bool pt)
    { pool_task_ = pt; }

   bool getTaskAllocType()
    { return pool_task_; }

   virtual void cleanup()
    {
    }

   // Fast requests can possibly take a shortcut and be serviced in the main
   // receiving thread; Note that a request that needs significant time to
   // determine if it is fast or not cannot be fast.
   virtual bool isFast () const
   { return false; }


   /// Called when the task gets the CPU
   virtual void run () = 0;

   Task (boost::function<void (Task *)> & resched)
      : status_ (0), reschedule_(resched)
   {
   }

   virtual ~Task () {}


   /// Called if the task can be rescheduled for work
   void reschedule ()
   { reschedule_ (this); }

protected:
   /// Called by the workqueue if we get the CPU
   virtual void doWork ();


protected:
   int status_;

   boost::function<void (Task *)> reschedule_;

   bool pool_task_;
};

//===========================================================================
}

#endif
