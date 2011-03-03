#ifndef IOFWDCLIENT_CBCLIENT_HH
#define IOFWDCLIENT_CBCLIENT_HH

#include "iofwdclient/iofwdclient-fwd.hh"
#include "iofwdclient/IOFWDClientCB.hh"

#include "zoidfs/zoidfs-async.h"

#include "iofwdutil/IOFWDLog-fwd.hh"
#include "iofwdutil/always_assert.hh"
#include "iofwdutil/tools.hh"

#include "sm/SMManager.hh"
#include "sm/SMClient.hh"

#include <boost/scoped_ptr.hpp>

namespace iofwdclient
{
   //========================================================================

   /**
    * Implements a callback version of the async zoidfs API
    */
   class CBClient
   {
      public:
         CBClient (iofwdutil::IOFWDLogSource & log,
               CommStream & net, bool poll = true);

         ~CBClient ();

      public:

         void cbgetattr (const IOFWDClientCB & cb,
               int * ret,
               const zoidfs::zoidfs_handle_t * handle,
               zoidfs::zoidfs_attr_t * attr,
               zoidfs::zoidfs_op_hint_t * op_hint);

         int cbsetattr(const IOFWDClientCB & cb,
                      int * ret,
                      const zoidfs::zoidfs_handle_t *handle,
                      const zoidfs::zoidfs_sattr_t *sattr,
                      zoidfs::zoidfs_attr_t *attr, 
                      zoidfs::zoidfs_op_hint_t * op_hint);

         int cblookup(const IOFWDClientCB & cb,
                     int * ret,
                     const zoidfs::zoidfs_handle_t *parent_handle,
                     const char *component_name, 
                     const char *full_path,
                     zoidfs::zoidfs_handle_t *handle,
                     zoidfs::zoidfs_op_hint_t * op_hint);
                     
         int cbreadlink(const IOFWDClientCB & cb,
                       int * ret,
                       const zoidfs::zoidfs_handle_t *handle,  
                       char *buffer,
                       size_t buffer_length,
                       zoidfs::zoidfs_op_hint_t * op_hint);
                       
         int cbcommit(const IOFWDClientCB & cb,
                     int * ret,
                     const zoidfs::zoidfs_handle_t *handle,
                     zoidfs::zoidfs_op_hint_t * op_hint);
                     
         int cbcreate(const IOFWDClientCB & cb,
                     int * ret,
                     const zoidfs::zoidfs_handle_t *parent_handle,
                     const char *component_name, 
                     const char *full_path,
                     const zoidfs::zoidfs_sattr_t *sattr, 
                     zoidfs::zoidfs_handle_t *handle,
                     int *created,
                     zoidfs::zoidfs_op_hint_t * op_hint);
                    
         int cbremove(const IOFWDClientCB & cb,
                     int * ret,
                     const zoidfs::zoidfs_handle_t *parent_handle,
                     const char *component_name, const char *full_path,
                     zoidfs::zoidfs_cache_hint_t *parent_hint,
                     zoidfs::zoidfs_op_hint_t * op_hint);
                     
         int cbrename(const IOFWDClientCB & cb,
                     int * ret,
                     const zoidfs::zoidfs_handle_t *from_parent_handle,
                     const char *from_component_name,
                     const char *from_full_path,
                     const zoidfs::zoidfs_handle_t *to_parent_handle,
                     const char *to_component_name,
                     const char *to_full_path,
                     zoidfs::zoidfs_cache_hint_t *from_parent_hint,
                     zoidfs::zoidfs_cache_hint_t *to_parent_hint,
                     zoidfs::zoidfs_op_hint_t * op_hint);
                     
         int cblink(const IOFWDClientCB & cb,
                   int * ret,
                   const zoidfs::zoidfs_handle_t *from_parent_handle,
                   const char *from_component_name,
                   const char *from_full_path,
                   const zoidfs::zoidfs_handle_t *to_parent_handle,
                   const char *to_component_name,
                   const char *to_full_path,
                   zoidfs::zoidfs_cache_hint_t *from_parent_hint,
                   zoidfs::zoidfs_cache_hint_t *to_parent_hint,
                   zoidfs::zoidfs_op_hint_t * op_hint);
                   
         int cbsymlink(const IOFWDClientCB & cb,
                      int * ret,
                      const zoidfs::zoidfs_handle_t *from_parent_handle,
                      const char *from_component_name,
                      const char *from_full_path,
                      const zoidfs::zoidfs_handle_t *to_parent_handle,
                      const char *to_component_name,
                      const char *to_full_path,
                      const zoidfs::zoidfs_sattr_t *sattr,
                      zoidfs::zoidfs_cache_hint_t *from_parent_hint,
                      zoidfs::zoidfs_cache_hint_t *to_parent_hint,
                      zoidfs::zoidfs_op_hint_t * op_hint);
                      
         int cbmkdir(const IOFWDClientCB & cb,
                    int * ret,
                    const zoidfs::zoidfs_handle_t *parent_handle,
                    const char *component_name, const char *full_path,
                    const zoidfs::zoidfs_sattr_t *sattr,
                    zoidfs::zoidfs_cache_hint_t *parent_hint,
                    zoidfs::zoidfs_op_hint_t * op_hint);
                            
         int cbreaddir(const IOFWDClientCB & cb,
                      int * ret,
                      const zoidfs::zoidfs_handle_t *parent_handle,
                      zoidfs::zoidfs_dirent_cookie_t cookie, size_t *entry_count_,
                      zoidfs::zoidfs_dirent_t *entries, uint32_t flags,
                      zoidfs::zoidfs_cache_hint_t *parent_hint,
                      zoidfs::zoidfs_op_hint_t * op_hint);

         int cbresize(const IOFWDClientCB & cb,
                     int * ret,
                     const zoidfs::zoidfs_handle_t *handle, 
                     zoidfs::zoidfs_file_size_t size,
                     zoidfs::zoidfs_op_hint_t * op_hint);
                     
                   
         int cbread(const IOFWDClientCB & cb,
                   int * ret,
                   const zoidfs::zoidfs_handle_t *handle, size_t mem_count,
                   void *mem_starts[], const size_t mem_sizes[],
                   size_t file_count, const zoidfs::zoidfs_file_ofs_t file_starts[],
                   zoidfs::zoidfs_file_size_t file_sizes[],
                   zoidfs::zoidfs_op_hint_t * op_hint);
                    
         int cbwrite(const IOFWDClientCB & cb,
                    int * ret,
                    const zoidfs::zoidfs_handle_t *handle, size_t mem_count,
                    const void *mem_starts[], const size_t mem_sizes[],
                    size_t file_count, const zoidfs::zoidfs_file_ofs_t file_starts[],
                    zoidfs::zoidfs_file_ofs_t file_sizes[],
                    zoidfs::zoidfs_op_hint_t * op_hint);


         int cbinit(const IOFWDClientCB & cb, int * ret, 
                    zoidfs::zoidfs_op_hint_t * op_hint);


         int cbfinalize(const IOFWDClientCB & cb, int * ret, 
                    zoidfs::zoidfs_op_hint_t * op_hint);

         int cbnull(const IOFWDClientCB & cb, int * ret, 
                    zoidfs::zoidfs_op_hint_t * op_hint);
      protected:

        class CBSMWrapper
        {
            public:
                static CBSMWrapper * createCBSMWrapper(const IOFWDClientCB & cb,
                        sm::SMClient * sm = NULL)
                {
                    return new CBSMWrapper(cb, sm);
                }

                void set(sm::SMClient * sm)
                {
                    sm_ = sm::SMClientSharedPtr(sm);
                }

                void call(zoidfs::zoidfs_comp_mask_t mask, const
                        iofwdevent::CBException & cbexception)
                {
                    cb_(mask, cbexception);
                }

                const IOFWDClientCB & getWCB()
                {
                    return wcb_;
                }

            protected:
                /* prevent stack allocation and copying of CBWrapper objects */
                CBSMWrapper(const IOFWDClientCB & cb,
                        sm::SMClient * sm = NULL) :
                    cb_(cb),
                    sm_(sm),
                    wcb_(boost::bind(&CBClient::cbWrapper, this, _1, _2))
                {
                }

                CBSMWrapper() : 
                    cb_(boost::bind(&CBSMWrapper::cbsentinel, this, _1, _2)),
                    sm_(NULL),
                    wcb_(boost::bind(&CBSMWrapper::cbsentinel, this, _1, _2))
                {
                }

                CBSMWrapper(const CBSMWrapper & rhs) :
                    cb_(rhs.cb_),
                    sm_(rhs.sm_),
                    wcb_(rhs.wcb_)
                {
                }

                CBSMWrapper & operator=(const CBSMWrapper & UNUSED(rhs))
                {
                    return *this;
                }

                /* empty callback... should never be invoked */
                void cbsentinel(zoidfs::zoidfs_comp_mask_t UNUSED(mask),
                        const iofwdevent::CBException & UNUSED(cbexception))
                {
                    ALWAYS_ASSERT(false && "CBSMWrapper::cbsentinel was invoked");
                }

                /* access to CBClient::cbWrapper */
                friend class CBClient;

                const IOFWDClientCB & cb_;
                sm::SMClientSharedPtr sm_;
                const IOFWDClientCB wcb_;
         };
        
        static void cbWrapper(CBSMWrapper * cbsm,
            zoidfs::zoidfs_comp_mask_t mask,
            const iofwdevent::CBException & cbexception);

      protected:
         iofwdutil::IOFWDLogSource & log_;
         CommStream & net_;
         bool poll_;

         boost::scoped_ptr<sm::SMManager> smm_;
   };

   //========================================================================
}

#endif
