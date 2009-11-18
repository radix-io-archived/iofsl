#include <iostream>
#include <boost/format.hpp>

using namespace boost;
using namespace std;

#include "iofwdutil/assert.hh"
#include "SMResourceOp.hh"
#include "SMManager.hh"
#include "SMResourceClient.hh"
#include "iofwdutil/IOFWDLog.hh"

namespace sm
{
//===========================================================================

SMResourceOp::SMResourceOp (SMManager * manager)
   : manager_(manager), completed_(1)
{
}

void SMResourceOp::cancel ()
{
   reschedule (false);
}

void SMResourceOp::success ()
{
   reschedule (true);
}

void SMResourceOp::reschedule (bool success)
{
   completed_ = 1;
   
   // @TODO: post event to state machine

   // Reset our client pointer but make sure its reference count doesn't 
   // go to zero.
   SMResourceClientSharedPtr ptr;
   ptr.swap (client_);

   // NOTE: need to lock the client here, otherwise it could be
   // scheduled twice between the call to setReschedule and schedule
   // It is not optimal since it prevents the worker thread from executing
   // 
   boost::mutex::scoped_lock l (ptr->getLock());

   // Note: calling completed on the client could cause it reschedule/rearm
   // with us; Don't assume anything about SMResourceOp members below!
   ptr->completed (success);

   /*std::cerr << boost::format("client %p refcount = %i") % ptr.get() % ptr->alive () << endl;*/
   if (ptr->setRescheduled ())
      manager_->schedule (ptr.get());
   /*std::cerr << boost::format("after reschedule client %p refcount = %i") % ptr.get() %
         ptr->alive () << endl;*/
}

void SMResourceOp::rearm (SMResourceClientSharedPtr client)
{
   client_ = client;
   completed_ = 0;
}

SMResourceOp::~SMResourceOp ()
{
   // If we destroy this before the success/cancel was called, 
   // we have a problem...
   ALWAYS_ASSERT(completed_);
}

//===========================================================================
}
