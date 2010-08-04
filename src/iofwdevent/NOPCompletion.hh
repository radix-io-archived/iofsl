#ifndef IOFWDEVENT_NOPCOMPLETION_HH
#define IOFWDEVENT_NOPCOMPLETION_HH

#include "iofwdevent/CBException.hh"


namespace iofwdevent
{
   //========================================================================


   /**
    * Callback that doesn't do anything.
    */
   struct NOPCompletion
   {
      void  operator  () (const CBException & e) const
      {
         e.check ();
      }
   };


   //========================================================================
}


#endif
