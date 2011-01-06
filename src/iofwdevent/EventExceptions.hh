#ifndef IOFWDEVENT_EVENTEXCEPTIONS_HH
#define IOFWDEVENT_EVENTEXCEPTIONS_HH

#include "iofwdutil/ZException.hh"
#include "Handle.hh"

namespace iofwdevent
{
   // =======================================================================

   struct EventException : virtual public iofwdutil::ZException {};

   /**
    * This exception passed to the callback when the cancel function was
    * called for a blocking operation.
    *
    * It contains an info structure holding the handle of the cancelled event.
    */
   struct EventCancelledException : virtual public EventException {};


   /**
    * The handle of the cancelled operation.
    * Used with EventCancelledException
    */
   typedef boost::error_info<struct tag_event_handle,iofwdevent::Handle>
      cancelled_handle;


   // =======================================================================
}

#endif
