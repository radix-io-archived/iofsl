#ifndef IOFWDEVENT_HANDLE_HH
#define IOFWDEVENT_HANDLE_HH

namespace iofwdevent
{
   //========================================================================

   /**
    * Each blocking resource operation returns a handle that can be used to
    * cancel the operation.
    *
    * @TODO: Go over the resources and make sure they all return valid handles
    * and support cancel.
    */
   typedef void * Handle;

   //========================================================================
}

#endif
