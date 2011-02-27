#ifndef IOFWDCLIENT_SYNCCLIENT_HH
#define IOFWDCLIENT_SYNCCLIENT_HH

#include "zoidfs/zoidfs.h"

#include "iofwdclient/iofwdclient-fwd.hh"
#include "iofwdclient/ASClient.hh"

#include "iofwdutil/IOFWDLog-fwd.hh"

namespace iofwdclient
{
   //========================================================================

   /**
    * Implements zoidfs normal API on top of the async API
    */
   class SyncClient
   {
      public:
         // this is zoidfs_init
         SyncClient (iofwdutil::IOFWDLogSource & log, ASClient & asclient);

         // this is zoidfs_finalize
         ~SyncClient ();

         // ------------- blocking ZoidFS functions -------------------------

         int getattr (const zoidfs::zoidfs_handle_t * handle,
               zoidfs::zoidfs_attr_t * attr,
               zoidfs::zoidfs_op_hint_t * op_hint);

      protected:

         int waitOp (zoidfs::zoidfs_request_t r, zoidfs::zoidfs_comp_mask_t);

      protected:
         iofwdutil::IOFWDLogSource & log_;
         ASClient & asclient_;
   };

   //========================================================================
}

#endif
