#ifndef IOFWDEVENT_RESOURCE_HH
#define IOFWDEVENT_RESOURCE_HH

#include "Handle.hh"

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

  typedef iofwdevent::Handle Handle;

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
    * It returns true if the operation could be cancelled, in which case the
    * callback associated with the operation is called with an exception
    * derived from EventCancelledException. cancel() /does not return/ before
    * the callback completed.
    *
    * If can no longer be cancelled, this function returns false.
    *
    * In either case, the callback will always be called, either during cancel
    * or, if cancel failed, when the operation completes (or completed).
    */
   virtual bool cancel (Handle h) = 0;

   virtual ~Resource ();

};
//===========================================================================
}

#endif
