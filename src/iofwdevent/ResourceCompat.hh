#ifndef IOFWDEVENT_RESOURCECOMPAT_HH
#define IOFWDEVENT_RESOURCECOMPAT_HH

#include "iofwdevent/ResourceOp.hh"
#include "iofwdevent/Resource.hh"

namespace iofwdevent
{

/**
 * This class calls the old callback functions for a ResourceOp objects.
 */
class ResourceCompatHelper
{
public:
   ResourceCompatHelper (ResourceOp & o)
      : op(o)
   {
   }

   void operator () (int status)
   {
      switch (status)
      {
         case Resource::COMPLETED:
            op.success ();
            break;
         case Resource::CANCELLED:
            op.cancel ();
            break;
         default:
            ALWAYS_ASSERT(false);
      }
   }

protected:
   ResourceOp & op;
};

ResourceCompatHelper resource_compat (ResourceOp & op)
{
   return ResourceCompatHelper (op);
}

}

#endif
