#ifndef IOFWDEVENT_RESOURCEWRAPPER_HH
#define IOFWDEVENT_RESOURCEWRAPPER_HH

#include "Resource.hh"

namespace iofwdevent
{
//===========================================================================

/**
 * This class makes sure stop is called on the resource.
 */
class ResourceWrapper
{
public:
   ResourceWrapper (Resource * res)
      : resource_(res)
   {
      if (!resource_->started ())
         resource_->start ();
   }

   ~ResourceWrapper ()
   {
      if (resource_->started())
         resource_->stop ();
   }

protected:
   Resource * resource_;
};
//===========================================================================
}

#endif
