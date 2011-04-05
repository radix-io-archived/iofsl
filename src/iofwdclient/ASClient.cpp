#include "iofwdclient/ASClient.hh"
#include "iofwdclient/CBClient.hh"
#include "iofwdclient/RequestTracker.hh"
#include "iofwdclient/IOFWDRequest.hh"

#include "iofwdutil/IOFWDLog.hh"
#include "iofwdutil/assert.hh"
#include "iofwd/RPCClient.hh"
#include <cstdio>
#include <boost/shared_ptr.hpp>
using namespace zoidfs;

namespace iofwdclient
{
   //========================================================================

   ASClient::ASClient (iofwdutil::IOFWDLogSource & log, CBClient & cb)
      : log_ (log),
        cbclient_ (cb),
        tracker_ (new RequestTracker ())
   {
      // We expect timeout to be at least a 32 bit type
      STATIC_ASSERT (sizeof (zoidfs_timeout_t) >= 4);
   }

   ASClient::~ASClient ()
   {
   }
   // @TODO: This needs to be changed, possibly use the CommStream class?
//   void ASClient::setRPCMode (boost::shared_ptr<iofwd::RPCClient> rpcclient,
//                              net::AddressPtr addr)
//   {
//      cbclient_.setRPCMode (rpcclient, addr);
//   }   

   IOFWDRequest * ASClient::getRequest (zoidfs::zoidfs_request_t req) const
   {
      return static_cast<IOFWDRequest*> (req);
   }

   int ASClient::request_test (zoidfs_request_t request,
         zoidfs_timeout_t timeout,
         zoidfs_comp_mask_t mask)
   {
      IOFWDRequest * ptr = getRequest (request);
      fprintf(stderr, "%s:%p\n", __func__,ptr);
      if (!tracker_->wait (ptr, mask, timeout))
         return ZFSERR_TIMEOUT;
      return ZFS_OK;
   }

   /**
    * We simply decrement the reference count for the request
    * and set the request handle to ZOIDFS_REQUEST_NULL.
    *
    * Returns ZFSERR_INVAL if request is 0, otherwise ZFS_OK.
    */
   int ASClient::request_free (zoidfs_request_t * request)
   {
      ALWAYS_ASSERT(request);

      if (! (*request))
         return ZFSERR_INVAL;

      IOFWDRequest * ptr = getRequest (request);
      tracker_->freeRequest (ptr);
      *request = ZOIDFS_REQUEST_NULL;
      return ZFS_OK;
   }

   int ASClient::request_get_comp_state (zoidfs_request_t request,
         zoidfs_comp_mask_t * mask)
   {
      if (request == ZOIDFS_REQUEST_NULL || !mask)
         return ZFSERR_INVAL;

      IOFWDRequest * ptr = getRequest (request);
      *mask = ptr->getCompletionStatus ();
      return ZFS_OK;
   }

   int ASClient::igetattr (zoidfs::zoidfs_request_t * request,
                   const zoidfs::zoidfs_handle_t * handle,
                   zoidfs::zoidfs_attr_t * attr,
                   zoidfs::zoidfs_op_hint_t * op_hint)
   {
      // validate arguments
      // Can op_hint be 0?
      if (*request || !handle || !attr)
         return ZFSERR_INVAL;

      // Create request
      //   newRequest automatically increments the refcount to compensate for
      //   the lack of automatic refcounting in the C API
      IOFWDRequestPtr r (tracker_->newRequest ());

      cbclient_.cbgetattr (tracker_->getCB (r),
                 r->getReturnPointer (),
                 handle, attr, op_hint);

      return ZFS_OK;
   }



   int ASClient::isetattr(zoidfs::zoidfs_request_t * request,
                             const zoidfs::zoidfs_handle_t *handle,
                             const zoidfs::zoidfs_sattr_t *sattr,
                             zoidfs::zoidfs_attr_t *attr, 
                             zoidfs::zoidfs_op_hint_t * op_hint)
   {
      // validate arguments
      // Can op_hint be 0?
      if (*request || !handle || !attr || !sattr)
         return ZFSERR_INVAL;

      // Create request
      //   newRequest automatically increments the refcount to compensate for
      //   the lack of automatic refcounting in the C API
      IOFWDRequestPtr r (tracker_->newRequest ());
      cbclient_.cbsetattr(tracker_->getCB (r),
                 r->getReturnPointer (),handle,sattr, attr, op_hint);   
      
      return ZFS_OK;
   }


   int ASClient::ilookup(   zoidfs::zoidfs_request_t * request,
                            const zoidfs::zoidfs_handle_t *parent_handle,
                            const char *component_name, 
                            const char *full_path,
                            zoidfs::zoidfs_handle_t *handle,
                            zoidfs::zoidfs_op_hint_t * op_hint)
   {  
      IOFWDRequest * r = tracker_->newRequest ();
      r->setCompletionStatus(zoidfs::ZFS_COMP_NONE);
      cbclient_.cblookup(tracker_->getCB (r), r->getReturnPointer (), 
                         parent_handle, component_name, full_path, handle, 
                         op_hint);
      //tmp = r;
      (*request) = (void *) r;
      return ZFS_OK;
   }
               
   int ASClient::ireadlink(zoidfs::zoidfs_request_t * request,
                              const zoidfs::zoidfs_handle_t *handle,  
                              char *buffer,   
                              size_t buffer_length,
                              zoidfs::zoidfs_op_hint_t * op_hint)
   {
            // validate arguments
      // Can op_hint be 0?
      if (*request || !handle || !buffer || !buffer_length)
         return ZFSERR_INVAL;

      // Create request
      //   newRequest automatically increments the refcount to compensate for
      //   the lack of automatic refcounting in the C API
      IOFWDRequestPtr r (tracker_->newRequest ());

      cbclient_.cbreadlink(tracker_->getCB (r), r->getReturnPointer (),
                           handle, buffer, buffer_length,  op_hint);
      return ZFS_OK;
   }
                 
   int ASClient::icommit(zoidfs::zoidfs_request_t * request,
                            const zoidfs::zoidfs_handle_t *handle,
                            zoidfs::zoidfs_op_hint_t * op_hint)
   {
            // validate arguments
      // Can op_hint be 0?
      if (*request || !handle )
         return ZFSERR_INVAL;

      // Create request
      //   newRequest automatically increments the refcount to compensate for
      //   the lack of automatic refcounting in the C API
      IOFWDRequestPtr r (tracker_->newRequest ());

      cbclient_.cbcommit( tracker_->getCB (r), r->getReturnPointer (), 
                          handle, op_hint);
      return ZFS_OK;
   }
               
   int ASClient::icreate(zoidfs::zoidfs_request_t * request,
                            const zoidfs::zoidfs_handle_t *parent_handle,
                            const char *component_name, 
                            const char *full_path,
                            const zoidfs::zoidfs_sattr_t *sattr, 
                            zoidfs::zoidfs_handle_t *handle,
                            int *created,
                            zoidfs::zoidfs_op_hint_t * op_hint)
   {
            // validate arguments
      // Can op_hint be 0?
      if (*request || !handle || !parent_handle || !component_name || 
          !full_path || !sattr || !created)
         return ZFSERR_INVAL;

      // Create request
      //   newRequest automatically increments the refcount to compensate for
      //   the lack of automatic refcounting in the C API
      IOFWDRequestPtr r (tracker_->newRequest ());

      cbclient_.cbcreate (tracker_->getCB (r), r->getReturnPointer (), 
                          parent_handle, component_name, full_path, sattr, 
                          handle, created, op_hint);
      return ZFS_OK;
   }
              
   int ASClient::iremove(zoidfs::zoidfs_request_t * request,
                            const zoidfs::zoidfs_handle_t *parent_handle,
                            const char *component_name, const char *full_path,
                            zoidfs::zoidfs_cache_hint_t *parent_hint,
                            zoidfs::zoidfs_op_hint_t * op_hint)
   {
            // validate arguments
      // Can op_hint be 0?
      if (*request || !parent_handle || !parent_hint || !full_path )
         return ZFSERR_INVAL;

      // Create request
      //   newRequest automatically increments the refcount to compensate for
      //   the lack of automatic refcounting in the C API
      IOFWDRequestPtr r (tracker_->newRequest ());

      cbclient_.cbremove (tracker_->getCB (r),
                          r->getReturnPointer (), parent_handle, component_name, 
                          full_path, parent_hint, op_hint);
      return ZFS_OK;
   }
               
   int ASClient::irename(zoidfs::zoidfs_request_t * request,
                            const zoidfs::zoidfs_handle_t *from_parent_handle,
                            const char *from_component_name,
                            const char *from_full_path,
                            const zoidfs::zoidfs_handle_t *to_parent_handle,
                            const char *to_component_name,
                            const char *to_full_path,
                            zoidfs::zoidfs_cache_hint_t *from_parent_hint,
                            zoidfs::zoidfs_cache_hint_t *to_parent_hint,
                            zoidfs::zoidfs_op_hint_t * op_hint)
   {
            // validate arguments
      // Can op_hint be 0?
      if (*request || !from_parent_handle || !from_component_name ||
          !from_full_path || !to_parent_handle || !to_component_name ||
          !to_full_path || !from_parent_hint || !to_parent_hint )
         return ZFSERR_INVAL;

      // Create request
      //   newRequest automatically increments the refcount to compensate for
      //   the lack of automatic refcounting in the C API
      IOFWDRequestPtr r (tracker_->newRequest ());

      cbclient_.cbrename ( tracker_->getCB (r), r->getReturnPointer (), 
                           from_parent_handle, 
                           from_component_name, from_full_path, 
                           to_parent_handle, to_component_name, 
                           to_full_path, from_parent_hint, 
                           to_parent_hint, op_hint);
      return ZFS_OK;
   }
               
   int ASClient::ilink(zoidfs::zoidfs_request_t * request,
                          const zoidfs::zoidfs_handle_t *from_parent_handle,
                          const char *from_component_name,
                          const char *from_full_path,
                          const zoidfs::zoidfs_handle_t *to_parent_handle,
                          const char *to_component_name,
                          const char *to_full_path,
                          zoidfs::zoidfs_cache_hint_t *from_parent_hint,
                          zoidfs::zoidfs_cache_hint_t *to_parent_hint,
                          zoidfs::zoidfs_op_hint_t * op_hint)
   {
            // validate arguments
      // Can op_hint be 0?
      if (*request || !from_parent_handle || !from_component_name ||
          !from_full_path || !to_parent_handle || !to_component_name ||
          !to_full_path || !from_parent_hint || !to_parent_hint )
         return ZFSERR_INVAL;

      // Create request
      //   newRequest automatically increments the refcount to compensate for
      //   the lack of automatic refcounting in the C API
      IOFWDRequestPtr r (tracker_->newRequest ());

      cbclient_.cblink(tracker_->getCB (r), r->getReturnPointer (), 
                       from_parent_handle, from_component_name,
                       from_full_path, to_parent_handle, 
                       to_component_name, to_full_path, 
                       from_parent_hint, to_parent_hint,
                       op_hint);   
      return ZFS_OK;
   }
             
   int ASClient::isymlink(zoidfs::zoidfs_request_t * request,
                             const zoidfs::zoidfs_handle_t *from_parent_handle,
                             const char *from_component_name,
                             const char *from_full_path,
                             const zoidfs::zoidfs_handle_t *to_parent_handle,
                             const char *to_component_name,
                             const char *to_full_path,
                             const zoidfs::zoidfs_sattr_t *sattr,
                             zoidfs::zoidfs_cache_hint_t *from_parent_hint,
                             zoidfs::zoidfs_cache_hint_t *to_parent_hint,
                             zoidfs::zoidfs_op_hint_t * op_hint)
   {
            // validate arguments
      // Can op_hint be 0?
      if (*request || !from_parent_handle || !from_component_name || !sattr ||
          !from_full_path || !to_parent_handle || !to_component_name ||
          !to_full_path || !from_parent_hint || !to_parent_hint )
         return ZFSERR_INVAL;

      // Create request
      //   newRequest automatically increments the refcount to compensate for
      //   the lack of automatic refcounting in the C API
      IOFWDRequestPtr r (tracker_->newRequest ());

      cbclient_.cbsymlink ( tracker_->getCB (r), r->getReturnPointer (), 
                            from_parent_handle, from_component_name,
                            from_full_path, to_parent_handle, 
                            to_component_name, to_full_path,
                            sattr, from_parent_hint, to_parent_hint,
                            op_hint);

      return ZFS_OK;
   }
   int ASClient::imkdir(zoidfs::zoidfs_request_t * request,
                           const zoidfs::zoidfs_handle_t *parent_handle,
                           const char *component_name, const char *full_path,
                           const zoidfs::zoidfs_sattr_t *sattr,
                           zoidfs::zoidfs_cache_hint_t *parent_hint,
                           zoidfs::zoidfs_op_hint_t * op_hint)
   {
            // validate arguments
      // Can op_hint be 0?
      if (*request || !parent_handle || !component_name || !full_path ||
          !sattr || !parent_hint)
         return ZFSERR_INVAL;

      // Create request
      //   newRequest automatically increments the refcount to compensate for
      //   the lack of automatic refcounting in the C API
      IOFWDRequestPtr r (tracker_->newRequest ());

      cbclient_.cbmkdir(tracker_->getCB (r), r->getReturnPointer (), 
                        parent_handle, component_name, full_path,
                        sattr, parent_hint, op_hint);
      return ZFS_OK;
   }
                      
   int ASClient::ireaddir(zoidfs::zoidfs_request_t * request,
                             const zoidfs::zoidfs_handle_t *parent_handle,
                             zoidfs::zoidfs_dirent_cookie_t cookie, size_t *entry_count_,
                             zoidfs::zoidfs_dirent_t *entries, uint32_t flags,
                             zoidfs::zoidfs_cache_hint_t *parent_hint,
                             zoidfs::zoidfs_op_hint_t * op_hint)
   {
            // validate arguments
      // Can op_hint be 0?
      if (*request || !parent_handle || !cookie || !entry_count_ || !entries
         || !flags || !parent_hint )
         return ZFSERR_INVAL;

      // Create request
      //   newRequest automatically increments the refcount to compensate for
      //   the lack of automatic refcounting in the C API
      IOFWDRequestPtr r (tracker_->newRequest ());

      cbclient_.cbreaddir(tracker_->getCB (r),
                          r->getReturnPointer (), parent_handle, cookie, 
                          entry_count_, entries, flags, parent_hint, op_hint);
      return ZFS_OK;
   }

   int ASClient::iresize(zoidfs::zoidfs_request_t * request,
                            const zoidfs::zoidfs_handle_t *handle, 
                            zoidfs::zoidfs_file_size_t size,
                            zoidfs::zoidfs_op_hint_t * op_hint)
   {
            // validate arguments
      // Can op_hint be 0?
      if (*request || !handle || !size)
         return ZFSERR_INVAL;

      // Create request
      //   newRequest automatically increments the refcount to compensate for
      //   the lack of automatic refcounting in the C API
      IOFWDRequestPtr r (tracker_->newRequest ());

      cbclient_.cbresize(tracker_->getCB (r),
                         r->getReturnPointer (), handle, size, op_hint);
      return ZFS_OK;
   }
               
             
   int ASClient::iread(zoidfs::zoidfs_request_t * request,
                          const zoidfs::zoidfs_handle_t *handle, size_t mem_count,
                          void *mem_starts[], const size_t mem_sizes[],
                          size_t file_count, 
                          const zoidfs::zoidfs_file_ofs_t file_starts[],
                          zoidfs::zoidfs_file_size_t file_sizes[],
                          zoidfs::zoidfs_op_hint_t * op_hint)
   {
            // validate arguments
      // Can op_hint be 0?
//      if (*request || !handle || !mem_count || !mem_starts || !mem_sizes ||
//          !file_count || !file_sizes)
//         return ZFSERR_INVAL;

      // Create request
      //   newRequest automatically increments the refcount to compensate for
      //   the lack of automatic refcounting in the C API
//      IOFWDRequestPtr r (tracker_->newRequest ());
      IOFWDRequest * r = tracker_->newRequest ();
      r->setCompletionStatus(zoidfs::ZFS_COMP_NONE);

      cbclient_.cbread(tracker_->getCB (r),
                       r->getReturnPointer (), handle, mem_count, mem_starts,
                       mem_sizes, file_count, file_starts, file_sizes, op_hint);
      (*request) = (void *) r;
      return ZFS_OK;
   }
              
   int ASClient::iwrite(zoidfs::zoidfs_request_t * request,
                         const zoidfs::zoidfs_handle_t *handle, size_t mem_count,
                         const void *mem_starts[], const size_t mem_sizes[],
                         size_t file_count, 
                         const zoidfs::zoidfs_file_ofs_t file_starts[],
                         zoidfs::zoidfs_file_ofs_t file_sizes[],
                         zoidfs::zoidfs_op_hint_t * op_hint)
   {
            // validate arguments
      // Can op_hint be 0?
//      if (*request || !handle || !mem_count || !mem_starts || !mem_sizes ||
//          !file_count || !file_sizes)
//         return ZFSERR_INVAL;

      IOFWDRequest * r = tracker_->newRequest ();
      r->setCompletionStatus(zoidfs::ZFS_COMP_NONE);

      cbclient_.cbwrite(tracker_->getCB (r),
                        r->getReturnPointer (), handle, mem_count, mem_starts, 
                        mem_sizes, file_count, file_starts, file_sizes, 
                        op_hint);
      (*request) = (void *) r;
      return ZFS_OK;
   }

   int ASClient::iinit (zoidfs::zoidfs_request_t * request,
                        zoidfs::zoidfs_op_hint_t * op_hint)
   {
      if (*request)
         return ZFSERR_INVAL;

      // Create request
      //   newRequest automatically increments the refcount to compensate for
      //   the lack of automatic refcounting in the C API
      IOFWDRequestPtr r (tracker_->newRequest ());


      cbclient_.cbinit(tracker_->getCB (r), r->getReturnPointer (), op_hint);
      return ZFS_OK;
   }

   int ASClient::ifinalize (zoidfs::zoidfs_request_t * request,
                            zoidfs::zoidfs_op_hint_t * op_hint)
   {
      if (*request)
         return ZFSERR_INVAL;

      // Create request
      //   newRequest automatically increments the refcount to compensate for
      //   the lack of automatic refcounting in the C API
      IOFWDRequestPtr r (tracker_->newRequest ());

      cbclient_.cbfinalize(tracker_->getCB (r), r->getReturnPointer (), 
                           op_hint);
      return ZFS_OK;

   }

   int ASClient::inull ( zoidfs::zoidfs_request_t * request,
                         zoidfs::zoidfs_op_hint_t * op_hint)
   {
      if (*request)
         return ZFSERR_INVAL;

      // Create request
      //   newRequest automatically increments the refcount to compensate for
      //   the lack of automatic refcounting in the C API
      IOFWDRequestPtr r (tracker_->newRequest ());

      cbclient_.cbnull(tracker_->getCB (r), r->getReturnPointer (), op_hint);
      return ZFS_OK;
   }
   //========================================================================
}
