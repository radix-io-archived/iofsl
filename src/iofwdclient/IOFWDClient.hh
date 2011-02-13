#ifndef IOFWDCLIENT_IOFWDCLIENT_HH
#define IOFWDCLIENT_IOFWDCLIENT_HH

#include "zoidfs/zoidfs.h"

#include "iofwdclient/ASClient.hh"


namespace iofwdclient
{
   //========================================================================

   class RequestTracker;

   /**
    * Implements zoidfs normal API on top of the async API
    *
    */
   class IOFWDClient : public ASClient
   {
      public:
         // this is zoidfs_init
         IOFWDClient ();

         // this is zoidfs_finalize
         ~IOFWDClient ();

         // ------------- blocking ZoidFS functions -------------------------

         int getattr (const zoidfs::zoidfs_handle_t * handle,
               zoidfs::zoidfs_attr_t * attr,
               zoidfs::zoidfs_op_hint_t * op_hint);

      protected:

         int waitOp (zoidfs::zoidfs_request_t r, zoidfs::zoidfs_comp_mask_t);
   };

   //========================================================================
}

#endif
