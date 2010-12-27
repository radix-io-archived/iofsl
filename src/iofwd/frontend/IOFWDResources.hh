#ifndef IOFWD_FRONTEND_IOFWDRESOURCES_HH
#define IOFWD_FRONTEND_IOFWDRESOURCES_HH

#include "iofwdutil/IOFWDLog-fwd.hh"

namespace iofwdevent
{
   class BMIResource;
}

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
   IOFWDResources (iofwdevent::BMIResource & bmi,
         iofwdutil::zlog::ZLogSource & l)
      : rbmi_(bmi), log_(l)
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
