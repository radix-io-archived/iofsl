#ifndef IOFWDCLIENT_IOFWDCLIENT_HH
#define IOFWDCLIENT_IOFWDCLIENT_HH

#include "zoidfs/zoidfs.h"
#include "zoidfs/zoidfs-async.h"

#include "iofwdclient/iofwdclient-fwd.hh"

#include "iofwdutil/IOFWDLog-fwd.hh"

#include <boost/scoped_ptr.hpp>

namespace iofwdclient
{
   //========================================================================

   /**
    * Implements ZoidFS API (sync + async)
    */
   class IOFWDClient
   {
      public:
         // this is zoidfs_init
         IOFWDClient ();

         // this is zoidfs_finalize
         ~IOFWDClient ();

         // -----------------------------------------------------------------
         // ------------- blocking ZoidFS functions -------------------------
         // -----------------------------------------------------------------

         int getattr (const zoidfs::zoidfs_handle_t * handle,
               zoidfs::zoidfs_attr_t * attr,
               zoidfs::zoidfs_op_hint_t * op_hint);

         // -----------------------------------------------------------------
         // -------------- zoidfs async methods -----------------------------
         // -----------------------------------------------------------------

         int igetattr(zoidfs::zoidfs_request_t * request,
                   const zoidfs::zoidfs_handle_t * handle,
                   zoidfs::zoidfs_attr_t * attr,
                   zoidfs::zoidfs_op_hint_t * op_hint);

         // -----------------------------------------------------------------
         // ------------------ Requests & Testing ---------------------------
         // -----------------------------------------------------------------

         int request_test (zoidfs::zoidfs_request_t request,
                   zoidfs::zoidfs_timeout_t timeout,
                   zoidfs::zoidfs_comp_mask_t mask);

         int request_get_error (zoidfs::zoidfs_request_t request, int * error);

         int request_get_comp_state (zoidfs::zoidfs_request_t,
               zoidfs::zoidfs_comp_mask_t *);

         int request_free (zoidfs::zoidfs_request_t * request);

      protected:
         iofwdutil::IOFWDLogSource & log_;

         boost::scoped_ptr<PollQueue>   pollqueue_;
         boost::scoped_ptr<ASClient>    asclient_;
         boost::scoped_ptr<SyncClient>  sclient_;
   };

   //========================================================================
}

#endif
