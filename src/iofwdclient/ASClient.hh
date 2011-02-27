#ifndef IOFWDCLIENT_ASCLIENT_HH
#define IOFWDCLIENT_ASCLIENT_HH

#include "iofwdclient/iofwdclient-fwd.hh"

#include "iofwdutil/IOFWDLog-fwd.hh"

#include "zoidfs/zoidfs-async.h"

#include <boost/scoped_ptr.hpp>

namespace iofwdclient
{
   //========================================================================

   class IOFWDRequest;

   /**
    * Implements the non-blocking ZoidFS API
    *
    * Takes a ZoidFS CB async implementation and builds the
    * non-blocking zoidfs functions on top of it.
    */
   class ASClient
   {
      public:
         ASClient (iofwdutil::IOFWDLogSource & log, CBClient & c);

         ~ASClient ();

         // -------------- zoidfs async methods ------------------

         int igetattr(zoidfs::zoidfs_request_t * request,
                   const zoidfs::zoidfs_handle_t * handle,
                   zoidfs::zoidfs_attr_t * attr,
                   zoidfs::zoidfs_op_hint_t * op_hint);

        // ------------------ Requests & Testing ----------------

         int request_test (zoidfs::zoidfs_request_t request,
                   zoidfs::zoidfs_timeout_t timeout,
                   zoidfs::zoidfs_comp_mask_t mask);

         int request_get_error (zoidfs::zoidfs_request_t request, int * error);

         int request_get_comp_state (zoidfs::zoidfs_request_t,
               zoidfs::zoidfs_comp_mask_t *);

         int request_free (zoidfs::zoidfs_request_t * request);

      protected:
         IOFWDRequest * getRequest (zoidfs::zoidfs_request_t req) const;

      protected:
         iofwdutil::IOFWDLogSource & log_;
         CBClient & cbclient_;
         boost::scoped_ptr<RequestTracker> tracker_;
   };

   //========================================================================
}

#endif
