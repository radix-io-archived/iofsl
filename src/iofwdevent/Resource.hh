#ifndef IOFWDEVENT_RESOURCE_HH
#define IOFWDEVENT_RESOURCE_HH

namespace iofwdevent
{
//===========================================================================

/**
 * Resource is anything that blocks.
 */
class Resource
{
public:

   /// Initialize resource, start any threads if required
   virtual void start () = 0;

   /// Prepare shutdown resource. Returns when shutdown complete
   /// No more requests will be completed after stop() returns.
   virtual void stop () = 0;

   virtual ~Resource ();

};
//===========================================================================
}

#endif
