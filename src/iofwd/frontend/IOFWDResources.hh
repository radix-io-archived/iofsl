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
      : rbmi_(r.rbmi_),
      log_(l)
   {
   }

   iofwdevent::BMIResource          &  rbmi_;

   /// Provides access to the frontend log
   iofwdutil::zlog::ZLogSource      &  log_;
};

//===========================================================================
   }
}

#endif
