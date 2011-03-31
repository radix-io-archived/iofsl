#ifndef IOFWDCLIENT_IOFWDREQUEST_HH
#define IOFWDCLIENT_IOFWDREQUEST_HH

#include "iofwdutil/IntrusiveHelper.hh"

#include "zoidfs/zoidfs-async.h"

#include <boost/intrusive_ptr.hpp>

namespace iofwdclient
{
   //========================================================================

   class RequestTracker;

   class IOFWDRequest : public iofwdutil::IntrusiveHelper
   {
      protected:
         friend class RequestTracker;

         IOFWDRequest (RequestTracker * tracker);

      public:

         bool hasError () const
         { return returncode_ != zoidfs::ZFS_OK; }


         int getReturnCode () const
         { return returncode_; }
   
         zoidfs::zoidfs_comp_mask_t getCompletionStatus () const
         { return completionstatus_; }

         /**
          * This is a convenience function, created so that the cbxxxx zoidfs
          * functions can directly update the status in the request
          */
         int * getReturnPointer ()
         { return &returncode_; }

         void setCompletionStatus (zoidfs::zoidfs_comp_mask_t comp)
         {
            // @TODO: assert only one of the completion state flags is set
            // (it is OK to have ZFS_COMP_ERROR in combination with some other
            // flags)
            completionstatus_ = comp;
         }

      protected:

         void setReturnCode (int ret)
         { returncode_ = ret; }

         RequestTracker * getTracker ()
         { return tracker_; }

      protected:
         int returncode_;
         zoidfs::zoidfs_comp_mask_t completionstatus_;

         RequestTracker * tracker_;

   };


   INTRUSIVE_PTR_HELPER(IOFWDRequest);

   typedef boost::intrusive_ptr<IOFWDRequest> IOFWDRequestPtr;

   //========================================================================
}

#endif
