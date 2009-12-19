#ifndef IOFWDEVENT_RESOURCE_HH
#define IOFWDEVENT_RESOURCE_HH

#include <boost/utility.hpp>
#include <boost/function.hpp>

namespace iofwdevent
{
//===========================================================================

/**
 * Resource is anything that blocks.
 *
 * TODO: Add exception suport
 */
class Resource : private boost::noncopyable
{
public:

   enum { COMPLETED = 0, CANCELLED, FAILED };

   typedef boost::function<void (int)> CBType;
   typedef void * Handle;

   /// Initialize resource, start any threads if required
   virtual void start () = 0;

   /// Prepare shutdown resource. Returns when shutdown complete
   /// No more requests will be completed after stop() returns.
   virtual void stop () = 0;

   /**
    * Return true if the resource has been started already.
    */
   virtual bool started () const = 0;

   /**
    * This function can be used to cancel a pending operation.
    */
   virtual bool cancel (Handle h) = 0;

   virtual ~Resource ();

};
//===========================================================================
}

#endif
