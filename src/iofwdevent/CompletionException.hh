#ifndef IOFWDEVENT_COMPLETIONEXCEPTION_HH
#define IOFWDEVENT_COMPLETIONEXCEPTION_HH

#include "iofwdutil/ZException.hh"

namespace iofwdevent
{
   //========================================================================

   // This exception gets thrown in MultiCompletion if a slot completes with
   // an exception
   struct SlotException : public virtual iofwdutil::ZException {};

   /* Error information tags for MultiCompletion */
   typedef boost::error_info<struct tag_slot_number, unsigned int> slot_number;

   //========================================================================
}

#endif
