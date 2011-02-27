#include "iofwdclient/IOFWDClient.hh"

#include "iofwdclient/PollQueue.hh"
#include "iofwdclient/ASClient.hh"
#include "iofwdclient/SyncClient.hh"

#include "iofwdutil/IOFWDLog.hh"

using namespace zoidfs;

namespace iofwdclient
{
   //========================================================================

   IOFWDClient::IOFWDClient ()
      : log_ (iofwdutil::IOFWDLog::getSource ())
   {
      ZLOG_DEBUG (log_, "Initializing IOFWDClient");
   }

   IOFWDClient::~IOFWDClient ()
   {
      ZLOG_DEBUG (log_, "Shutting down IOFWDClient");
   }
   
   // @TODO: repeat for other functions

   int IOFWDClient::getattr (const zoidfs_handle_t * handle,
         zoidfs_attr_t * attr, zoidfs_op_hint_t * op_hint)
   {
      return sclient_->getattr (handle, attr, op_hint);
   }


   int IOFWDClient::igetattr(zoidfs::zoidfs_request_t * request,
                   const zoidfs::zoidfs_handle_t * handle,
                   zoidfs::zoidfs_attr_t * attr,
                   zoidfs::zoidfs_op_hint_t * op_hint)
   {
      return asclient_->igetattr (request, handle, attr, op_hint);
   }


   int IOFWDClient::request_test (zoidfs::zoidfs_request_t request,
                   zoidfs::zoidfs_timeout_t timeout,
                   zoidfs::zoidfs_comp_mask_t mask)
   {
      return asclient_->request_test (request, timeout, mask);
   }

   int IOFWDClient::request_get_error (zoidfs::zoidfs_request_t request,
         int * error)
   {
      return asclient_->request_get_error (request, error);
   }

   int IOFWDClient::request_get_comp_state (zoidfs::zoidfs_request_t r,
               zoidfs::zoidfs_comp_mask_t * m)
   {
      return asclient_->request_get_comp_state (r, m);
   }

   int IOFWDClient::request_free (zoidfs::zoidfs_request_t * request)
   {
      return asclient_->request_free (request);
   }


   //========================================================================
}
