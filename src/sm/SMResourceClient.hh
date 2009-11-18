#ifndef SM_SMRESOURCECLIENT_HH
#define SM_SMRESOURCECLIENT_HH

#include <csignal>
#include <boost/thread.hpp>
#include "iofwdutil/IntrusiveHelper.hh"
#include "SMClient.hh"

namespace sm
{
//===========================================================================

class SMResourceClient : public SMClient
{
public:
   /// This method is called when the ResourceOp completes.
   /// Since it is called from within the resource thread, 
   /// it should not block and do as little work as possible.
   /// This method is called *before* the client is put
   /// back onto the work queue.
   /// If concurrent operations are allowed, the bool needs to be upgraded to 
   /// a type to distinguish which operation completed.
   virtual void completed (bool success) = 0;

   /// This method is called from a worker thread and (while it shouldn't
   /// block) can perform more work. It has the same function as the SMClient
   /// execute method.
   virtual bool executeClient () = 0;

   /// We implement execute and clear the reschedule flag before calling
   /// executeClient.
   virtual bool execute ()
   { clearRescheduled (); return executeClient (); }

   /// This method is called when the client is placed back onto the execution
   /// queue. It returns false if it was already scheduled for execution.
   /// Caller needs to lock on getLock () first.
   bool setRescheduled ()
   {
      // This is OK, since our caller needs to lock before calling this
      // method. There is no race here.
      if (rescheduled_)
         return false;
      rescheduled_ = 1;
      return true;
   }

   void clearRescheduled ()
   {
      rescheduled_ = 0;
   }

   boost::mutex &  getLock ()
   { return lock_; }

   SMResourceClient ()
      : rescheduled_(0)
   {
   }

protected:
      sig_atomic_t rescheduled_;

      boost::mutex lock_;
};

typedef boost::intrusive_ptr<SMResourceClient> SMResourceClientSharedPtr;


//===========================================================================
}
#endif
