#ifndef IOFWDCLIENT_CBCLIENT_HH
#define IOFWDCLIENT_CBCLIENT_HH

#include "iofwdclient/iofwdclient-fwd.hh"
#include "iofwdclient/IOFWDClientCB.hh"

#include "iofwdclient/CommStream.hh"

#include "zoidfs/zoidfs-async.h"

#include "iofwdutil/IOFWDLog-fwd.hh"
#include "iofwdutil/always_assert.hh"
#include "iofwdutil/tools.hh"
#include "iofwdutil/IOFWDLog.hh"

#include "sm/SMManager.hh"
#include "sm/SMClient.hh"

#include <boost/scoped_ptr.hpp>
#include "iofwd/RPCClient.hh"
#include "rpc/RPCHandler.hh"
#include <boost/shared_ptr.hpp>

#include <boost/intrusive_ptr.hpp>
#include <cstdio>

#include "iofwdclient/clientsm/GetAttrClientSM.hh"
#include "iofwdclient/clientsm/LookupClientSM.hh"
#include "iofwdclient/clientsm/WriteClientSM.hh"
#include "iofwdclient/clientsm/ReadClientSM.hh"
#include "iofwdclient/clientsm/CreateClientSM.hh"
#include "iofwdclient/clientsm/CommitClientSM.hh"

#include "common/rpc/CommonRequest.hh"
#include "iofwdclient/clientsm/RPCCommClientSM.hh"
#include "iofwdclient/clientsm/RPCCommWriteSM.hh"
#include "iofwdclient/clientsm/RPCCommReadSM.hh"
namespace iofwdclient
{
   typedef iofwdclient::clientsm::RPCCommClientSM<common::RPCCommitRequest, common::RPCCommitResponse> RPCCommCommit;
   typedef iofwdclient::clientsm::RPCCommClientSM<common::RPCGetAttrRequest, common::RPCGetAttrResponse> RPCCommGetAttr;
   typedef iofwdclient::clientsm::RPCCommClientSM<common::RPCCreateRequest, common::RPCCreateResponse> RPCCommClientSMCreate;
   typedef iofwdclient::clientsm::RPCCommClientSM<common::RPCLookupRequest, common::RPCLookupResponse> RPCCommClientSMLookup;
   typedef iofwdclient::clientsm::RPCCommWriteSM<common::RPCWriteRequest, common::RPCWriteResponse> RPCCommClientSMWrite;
   typedef iofwdclient::clientsm::RPCCommReadSM<common::RPCReadRequest, common::RPCReadResponse> RPCCommClientSMRead;
   /**
    * Implements a callback version of the async zoidfs API
    */
   class CBClient
   {
      public:
         CBClient (iofwdutil::IOFWDLogSource & log,
               CommStream & net, net::AddressPtr addr, bool poll = true);

         ~CBClient ();

      public:
         void cbgetattr (const IOFWDClientCB & cb,
               int * ret,
               const zoidfs::zoidfs_handle_t * handle,
               zoidfs::zoidfs_attr_t * attr,
               zoidfs::zoidfs_op_hint_t * op_hint);

         void cbsetattr(const IOFWDClientCB & UNUSED(cb),
                      int * UNUSED(ret),
                      const zoidfs::zoidfs_handle_t * UNUSED(handle),
                      const zoidfs::zoidfs_sattr_t * UNUSED(sattr),
                      zoidfs::zoidfs_attr_t * UNUSED(attr), 
                      zoidfs::zoidfs_op_hint_t * UNUSED(op_hint)) {ASSERT ("SetAttr Not Implemented" != 0); }

         void cblookup(const IOFWDClientCB & cb,
                     int * ret,
                     const zoidfs::zoidfs_handle_t *parent_handle,
                     const char *component_name, 
                     const char *full_path,
                     zoidfs::zoidfs_handle_t *handle,
                     zoidfs::zoidfs_op_hint_t * op_hint);
                     
         void cbreadlink(const IOFWDClientCB & UNUSED(cb),
                       int * UNUSED(ret),
                       const zoidfs::zoidfs_handle_t * UNUSED(handle),  
                       char * UNUSED(buffer),
                       size_t UNUSED(buffer_length),
                       zoidfs::zoidfs_op_hint_t * UNUSED(op_hint)) {ASSERT ("ReadLink Not Implemented" != 0);}
                       
         void cbcommit(const IOFWDClientCB & cb,
                     int * ret,
                     const zoidfs::zoidfs_handle_t * handle,
                     zoidfs::zoidfs_op_hint_t * op_hint);
                     
         void cbcreate(const IOFWDClientCB & cb,
                     int * ret,
                     const zoidfs::zoidfs_handle_t *parent_handle,
                     const char *component_name, 
                     const char *full_path,
                     const zoidfs::zoidfs_sattr_t *sattr, 
                     zoidfs::zoidfs_handle_t *handle,
                     int *created,
                     zoidfs::zoidfs_op_hint_t * op_hint);
                    
         void cbremove(const IOFWDClientCB & UNUSED(cb),
                     int * UNUSED(ret),
                     const zoidfs::zoidfs_handle_t * UNUSED(parent_handle),
                     const char * UNUSED(component_name), const char * UNUSED(full_path),
                     zoidfs::zoidfs_cache_hint_t * UNUSED(parent_hint),
                     zoidfs::zoidfs_op_hint_t * UNUSED(op_hint)) {ASSERT ("Remove Not Implemented" != 0); }
                     
         void cbrename(const IOFWDClientCB & UNUSED(cb),
                     int * UNUSED(ret),
                     const zoidfs::zoidfs_handle_t * UNUSED(from_parent_handle),
                     const char * UNUSED(from_component_name),
                     const char * UNUSED(from_full_path),
                     const zoidfs::zoidfs_handle_t * UNUSED(to_parent_handle),
                     const char * UNUSED(to_component_name),
                     const char * UNUSED(to_full_path),
                     zoidfs::zoidfs_cache_hint_t * UNUSED(from_parent_hint),
                     zoidfs::zoidfs_cache_hint_t * UNUSED(to_parent_hint),
                     zoidfs::zoidfs_op_hint_t * UNUSED(op_hint)) {ASSERT ("Rename Not Implemented" != 0); }
                     
         void cblink(const IOFWDClientCB & UNUSED(cb),
                   int * UNUSED(ret),
                   const zoidfs::zoidfs_handle_t * UNUSED(from_parent_handle),
                   const char * UNUSED(from_component_name),
                   const char * UNUSED(from_full_path),
                   const zoidfs::zoidfs_handle_t * UNUSED(to_parent_handle),
                   const char * UNUSED(to_component_name),
                   const char * UNUSED(to_full_path),
                   zoidfs::zoidfs_cache_hint_t * UNUSED(from_parent_hint),
                   zoidfs::zoidfs_cache_hint_t * UNUSED(to_parent_hint),
                   zoidfs::zoidfs_op_hint_t * UNUSED(op_hint)) {ASSERT ("Link Not Implemented" != 0); }
                   
         void cbsymlink(const IOFWDClientCB & UNUSED(cb),
                      int * UNUSED(ret),
                      const zoidfs::zoidfs_handle_t * UNUSED(from_parent_handle),
                      const char * UNUSED(from_component_name),
                      const char * UNUSED(from_full_path),
                      const zoidfs::zoidfs_handle_t * UNUSED(to_parent_handle),
                      const char * UNUSED(to_component_name),
                      const char * UNUSED(to_full_path),
                      const zoidfs::zoidfs_sattr_t * UNUSED(sattr),
                      zoidfs::zoidfs_cache_hint_t * UNUSED(from_parent_hint),
                      zoidfs::zoidfs_cache_hint_t * UNUSED(to_parent_hint),
                      zoidfs::zoidfs_op_hint_t * UNUSED(op_hint)) {ASSERT ("symlink Not Implemented" != 0); }
                      
         void cbmkdir(const IOFWDClientCB & UNUSED(cb),
                    int * UNUSED(ret),
                    const zoidfs::zoidfs_handle_t * UNUSED(parent_handle),
                    const char * UNUSED(component_name), const char * UNUSED(full_path),
                    const zoidfs::zoidfs_sattr_t * UNUSED(sattr),
                    zoidfs::zoidfs_cache_hint_t * UNUSED(parent_hint),
                    zoidfs::zoidfs_op_hint_t * UNUSED(op_hint)) {ASSERT ("mkdir Not Implemented" != 0); }
                            
         void cbreaddir(const IOFWDClientCB & UNUSED(cb),
                      int * UNUSED(ret),
                      const zoidfs::zoidfs_handle_t * UNUSED(parent_handle),
                      zoidfs::zoidfs_dirent_cookie_t UNUSED(cookie), size_t * UNUSED(entry_count_),
                      zoidfs::zoidfs_dirent_t * UNUSED(entries), uint32_t UNUSED(flags),
                      zoidfs::zoidfs_cache_hint_t * UNUSED(parent_hint),
                      zoidfs::zoidfs_op_hint_t * UNUSED(op_hint)) {ASSERT ("readdir Not Implemented" != 0); }

         void cbresize(const IOFWDClientCB & UNUSED(cb),
                     int * UNUSED(ret),
                     const zoidfs::zoidfs_handle_t * UNUSED(handle), 
                     zoidfs::zoidfs_file_size_t UNUSED(size),
                     zoidfs::zoidfs_op_hint_t * UNUSED(op_hint)) {ASSERT ("Resize Not Implemented" != 0); }
                     
                   
         void cbread(const IOFWDClientCB & cb,
                   int * ret,
                   const zoidfs::zoidfs_handle_t *handle, size_t mem_count,
                   void *mem_starts[], const size_t mem_sizes[],
                   size_t file_count, const zoidfs::zoidfs_file_ofs_t file_starts[],
                   zoidfs::zoidfs_file_size_t file_sizes[],
                   zoidfs::zoidfs_op_hint_t * op_hint);
                    
         void cbwrite(const IOFWDClientCB & cb,
                    int * ret,
                    const zoidfs::zoidfs_handle_t *handle, size_t mem_count,
                    const void *mem_starts[], const size_t mem_sizes[],
                    size_t file_count, const zoidfs::zoidfs_file_ofs_t file_starts[],
                    zoidfs::zoidfs_file_ofs_t file_sizes[],
                    zoidfs::zoidfs_op_hint_t * op_hint);


         void cbinit(const IOFWDClientCB & UNUSED(cb), int * UNUSED(ret), 
                    zoidfs::zoidfs_op_hint_t * UNUSED(op_hint)) {ASSERT ("init Not Implemented" != 0); }


         void cbfinalize(const IOFWDClientCB & UNUSED(cb), int * UNUSED(ret), 
                    zoidfs::zoidfs_op_hint_t * UNUSED(op_hint)) {ASSERT ("Finalize Not Implemented" != 0); }

         void cbnull(const IOFWDClientCB & UNUSED(cb), int * UNUSED(ret), 
                    zoidfs::zoidfs_op_hint_t * UNUSED(op_hint)) {ASSERT ("null Not Implemented" != 0); }
      protected:

        class CBSMWrapper
        {
            public:
                static CBSMWrapper * createCBSMWrapper(const IOFWDClientCB & cb,
                        sm::SMClientSharedPtr sm = NULL)
                {
                    return new CBSMWrapper(cb, sm);
                }

                void set(sm::SMClientSharedPtr sm)
                {
                    sm_ = sm;
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
                        sm::SMClientSharedPtr sm = NULL) :
                    sm_(sm),
                    wcb_(boost::bind(&CBClient::cbWrapper, this, _1, _2))
                {
                   cb_ = cb;
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

                /* empty callback... should never be invoked */
                void cbsentinel(zoidfs::zoidfs_comp_mask_t UNUSED(mask),
                        const iofwdevent::CBException & UNUSED(cbexception))
                {
                    ALWAYS_ASSERT(false && "CBSMWrapper::cbsentinel was invoked");
                }

                /* access to CBClient::cbWrapper */
                friend class CBClient;

                IOFWDClientCB cb_;
                sm::SMClientSharedPtr sm_;
                const IOFWDClientCB wcb_;
         };
        
        static void cbWrapper(CBSMWrapper * cbsm,
            zoidfs::zoidfs_comp_mask_t mask,
            const iofwdevent::CBException & cbexception);

      protected:
         iofwdutil::IOFWDLogSource & log_;
         CommStream & net_;
         net::AddressPtr addr_;
         bool poll_;
         boost::shared_ptr<iofwd::RPCClient> client_;
         boost::shared_ptr<sm::SMManager> smm_;
   };

   //========================================================================
}

#endif
