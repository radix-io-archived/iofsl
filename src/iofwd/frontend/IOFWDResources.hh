#ifndef IOFWD_FRONTEND_IOFWDRESOURCES_HH
#define IOFWD_FRONTEND_IOFWDRESOURCES_HH

#include "iofwd/Resources.hh"
#include "iofwdutil/completion/BMIResource.hh"
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
   IOFWDResources (Resources & r, iofwdutil::completion::BMIResource & bmi,
         iofwdutil::zlog::ZLogSource & l)
      : rtimer_(r.rtimer_), rbmi_(r.rbmi_), rtoken_(r.rtoken_),
      bmires_(bmi),
      log_(l)
   {
   }

   iofwdevent::TimerResource        &  rtimer_;
   iofwdevent::BMIResource          &  rbmi_;
   iofwdevent::TokenResource        &  rtoken_;

   // This needs to go once we migrate fully to resources
   iofwdutil::completion::BMIResource & bmires_;
   iofwdutil::bmi::BMIContextPtr        bmictx_;

   /// Provides access to the frontend log
   iofwdutil::zlog::ZLogSource      &  log_;
};

//===========================================================================
   }
}

#endif
