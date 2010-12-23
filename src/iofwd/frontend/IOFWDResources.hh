#ifndef IOFWD_FRONTEND_IOFWDRESOURCES_HH
#define IOFWD_FRONTEND_IOFWDRESOURCES_HH

#include "iofwd/Resources.hh"
#include "iofwdutil/IOFWDLog.hh"

namespace iofwd
{
   namespace frontend
   {
//===========================================================================

/**
 * This class is used as a  way to group variables that need to be
 * passed to all the concrete IOFWD request implementations.
 *
 */
struct IOFWDResources
{
   IOFWDResources (Resources & r,
         iofwdutil::zlog::ZLogSource & l)
      : rtimer_(r.rtimer_), rbmi_(r.rbmi_), rtoken_(r.rtoken_),
      log_(l)
   {
   }

   iofwdevent::TimerResource        &  rtimer_;
   iofwdevent::BMIResource          &  rbmi_;
   iofwdevent::TokenResource        &  rtoken_;

   /// Provides access to the frontend log
   iofwdutil::zlog::ZLogSource      &  log_;
};

//===========================================================================
   }
}

#endif
