#ifndef IOFWDUTIL_WORKQUEUE_FUTURE_HH
#define IOFWDUTIL_WORKQUEUE_FUTURE_HH

#include <boost/shared_ptr.hpp>
#include <boost/type_traits.hpp>
#include <boost/bind.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/condition.hpp>

namespace iofwdutil
{
   namespace workqueue
   {
//===========================================================================

// TODO: make workqueue implementations policy based (policy on what happens
// on job init, execution and completion and which value is returned from
// queueWork)

// Next, make policy for returning futures or for registering in
// completiontracker.


//===========================================================================
   }
}

#endif
