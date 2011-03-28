#include "iofwdclient/IOFWDClient.hh"
#include "iofwdutil/assert.hh"

#include "src/zoidfs/zoidfs.h"
#include "src/zoidfs/zoidfs-async.h"
#include "src/iofwdclient/iofwdclientlib.hh"
#include <stdio.h>
using namespace zoidfs;

namespace iofwdclient
{
   //========================================================================

   /**
    * We hide all implementation in the IOFWDClient class, which is explicitly
    * initialized by zoidfs_init() to avoid any C/C++ linker interoperability
    * problems.
    */
   static IOFWDClient * client_     = 0;
   static unsigned int  client_ref_ = 0;

   extern "C"
   {
      //=====================================================================

      //---------------------------------------------------------------------
      //---- zoidfs.h functions ---------------------------------------------
      //---------------------------------------------------------------------

      int zoidfs_init (void)
      {
         if (!client_ref_)
            client_ = new IOFWDClient ();
         ++client_ref_;
         return zoidfs::ZFS_OK;
      }

      int zoidfs_finalize (void)
      {
         ALWAYS_ASSERT(client_ref_);
         --client_ref_;
         if (!client_ref_)
         {
            delete client_;
            client_ = 0;
         }
         return zoidfs::ZFS_OK;
      }
      
      // @TODO implement other blocking functions in a similar fashion

      int zoidfs_getattr(const zoidfs::zoidfs_handle_t * handle,
                   zoidfs::zoidfs_attr_t * attr,
                   zoidfs::zoidfs_op_hint_t * op_hint)
      {
         ASSERT(client_);
         return client_->getattr (handle, attr, op_hint);
      }

      int zoidfs_lookup(const zoidfs::zoidfs_handle_t *parent_handle,
                       const char *component_name, 
                       const char *full_path,
                       zoidfs::zoidfs_handle_t *handle,
                       zoidfs::zoidfs_op_hint_t * op_hint)
      {
//         ASSERT(client_);
         printf("HERE");
         return client_->lookup (parent_handle, component_name, full_path,
                                 handle, op_hint);
      }                              

      //---------------------------------------------------------------------
      //---- zoidfs-async.h functions ---------------------------------------
      //---------------------------------------------------------------------

      int zoidfs_igetattr(zoidfs_request_t * request,
                   const zoidfs_handle_t * handle,
                   zoidfs_attr_t * attr,
                   zoidfs_op_hint_t * op_hint)
      {
         return client_->igetattr (request, handle, attr, op_hint);
      }

      int zoidfs_request_test(zoidfs_request_t request,
            zoidfs_timeout_t timeout,
            zoidfs_comp_mask_t mask)
      {
         ASSERT(client_);
         return client_->request_test (request, timeout, mask);
      }

      int zoidfs_request_free (zoidfs_request_t * request)
      {
         ASSERT(client_);
         return client_->request_free (request);
      }

      int zoidfs_request_get_error (zoidfs_request_t request, int * error)
      {
         ASSERT(client_);
         return client_->request_get_error (request, error);
      }

      int zoidfs_request_get_comp_state (zoidfs_request_t request,
            zoidfs_comp_mask_t * state)
      {
         ASSERT(client_);
         return client_->request_get_comp_state (request, state);
      }

      //=====================================================================
   }
   //========================================================================
}
