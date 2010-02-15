#ifndef IOFWDEVENT_NOPCOMPLETION_HH
#define IOFWDEVENT_NOPCOMPLETION_HH

namespace iofwdevent
{
   //========================================================================


   /**
    * Callback that doesn't do anything.
    */
   struct NOPCompletion
   {
      void  operator  () (int) const
      {
      }
   };


   //========================================================================
}


#endif
