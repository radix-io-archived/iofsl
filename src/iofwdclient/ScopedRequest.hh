#ifndef IOFWDCLIENT_SCOPEDREQUEST_HH
#define IOFWDCLIENT_SCOPEDREQUEST_HH

#include "zoidfs-async.h"
#include "iofwdclient/iofwdclientlib.hh"
namespace iofwdclient
{
   //========================================================================

   /**
    * Helper class similar to scoped_ptr; Automatically releases the
    * zoidfs_request_t when going out of scope.
    */
   class ScopedRequest
   {
      protected:
         typedef zoidfs::zoidfs_request_t zoidfs_request_t;

      public:

         ScopedRequest (zoidfs_request_t req)
            : req_ (req)
         {
         }

         operator zoidfs_request_t () const
         { return req_; }

         ~ScopedRequest ()
         {
            /* Free'd elsewhere (SyncClient/ASClient) */
            //zoidfs::zoidfs_request_free (&req_);
         }

      protected:
         zoidfs_request_t req_;
   };

   //========================================================================
}

#endif
