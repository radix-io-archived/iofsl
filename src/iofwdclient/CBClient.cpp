#include "iofwdclient/CBClient.hh"
//using namespace zoidfs;

namespace iofwdclient
{
   CBClient::CBClient (iofwdutil::IOFWDLogSource & log,
         CommStream & net, net::AddressPtr addr, bool poll)
      : log_ (log),
        net_ (net),
        addr_(addr),
        poll_(poll),
        client_(iofwd::service::ServiceManager::instance().loadService<iofwd::RPCClient>("rpcclient")),
        smm_(new sm::SMManager(poll))
   {
   }

   CBClient::~CBClient ()
   {
   }


   void CBClient::cbWrapper(CBSMWrapper * cbsm,
           zoidfs::zoidfs_comp_mask_t mask,
           const iofwdevent::CBException & cbexception)
   {
       /* call the original cb */
       
       cbsm->call(mask, cbexception);

       /* delete the wrapper... trigger ref count dec on SM */
       delete cbsm;
   }


   void CBClient::cbcommit(const IOFWDClientCB & cb, int * ret,
                           const zoidfs::zoidfs_handle_t * handle,
                           zoidfs::zoidfs_op_hint_t * op_hint)
  {
     /* create the empty wrapper */
     CBSMWrapper * cbsm = CBSMWrapper::createCBSMWrapper(cb);

     /* Sets up the handler for the RPC State Machine */
     /* Should be changed to RPC KEY */
     rpc::RPCClientHandle rpc_handle = client_->rpcConnect(ZOIDFS_COMMIT_RPC.c_str(), addr_);
     boost::shared_ptr<RPCCommCommit> comm;
     comm.reset(new RPCCommCommit (smm_, rpc_handle, poll_));
     
     /* create the state machine */
     sm::SMClientSharedPtr sm(new iofwdclient::clientsm::CommitClientSM(*smm_, poll_, comm, 
                              cbsm->getWCB(), ret, handle, op_hint));
    
     /* add the sm to the cb wrapper */ 
     cbsm->set(sm);

     /* execute the sm */
     sm->execute();      
  }


   void CBClient::cbgetattr(const IOFWDClientCB & cb,
         int * ret,
         const zoidfs::zoidfs_handle_t * handle,
         zoidfs::zoidfs_attr_t * attr,
         zoidfs::zoidfs_op_hint_t * op_hint)
   {
       /* create the empty wrapper */
       CBSMWrapper * cbsm = CBSMWrapper::createCBSMWrapper(cb);

       /* Sets up the handler for the RPC State Machine */
       /* Should be changed to RPC KEY */
       rpc::RPCClientHandle rpc_handle = client_->rpcConnect(ZOIDFS_GETATTR_RPC.c_str(), addr_);
       boost::shared_ptr<RPCCommGetAttr> comm;
       comm.reset(new RPCCommGetAttr (smm_, rpc_handle, poll_));

       /* create the state machine */
       sm::SMClientSharedPtr sm(new iofwdclient::clientsm::GetAttrClientSM(*smm_, poll_, comm,
                                cbsm->getWCB(), ret, handle, attr, op_hint));
      
       /* add the sm to the cb wrapper */ 
       cbsm->set(sm);

       /* execute the sm */
       sm->execute();
   }

                     
  void CBClient::cblink(const IOFWDClientCB & cb,
              int * ret,
              const zoidfs::zoidfs_handle_t * from_parent_handle,
              const char * from_component_name,
              const char * from_full_path,
              const zoidfs::zoidfs_handle_t * to_parent_handle,
              const char * to_component_name,
              const char * to_full_path,
              zoidfs::zoidfs_cache_hint_t * from_parent_hint,
              zoidfs::zoidfs_cache_hint_t * to_parent_hint,
              zoidfs::zoidfs_op_hint_t * op_hint)
  {
       /* create the empty wrapper */
       CBSMWrapper * cbsm = CBSMWrapper::createCBSMWrapper(cb);

       /* Sets up the handler for the RPC State Machine */
       /* Should be changed to RPC KEY */
       rpc::RPCClientHandle rpc_handle = client_->rpcConnect(ZOIDFS_LINK_RPC.c_str(), addr_);
       boost::shared_ptr<RPCCommLink> comm;
       comm.reset(new RPCCommLink (smm_, rpc_handle, poll_));

       /* create the state machine */
       sm::SMClientSharedPtr sm(new iofwdclient::clientsm::LinkClientSM(*smm_, poll_, comm,
                                cbsm->getWCB(), ret, from_parent_handle, from_component_name,
                                from_full_path, to_parent_handle, to_component_name, to_full_path,
                                from_parent_hint, to_parent_hint, op_hint));
      
       /* add the sm to the cb wrapper */ 
       cbsm->set(sm);

       /* execute the sm */
       sm->execute();
  }
  void CBClient::cbsetattr(const IOFWDClientCB & cb,
                int * ret,
                const zoidfs::zoidfs_handle_t * handle,
                const zoidfs::zoidfs_sattr_t * sattr,
                zoidfs::zoidfs_attr_t * attr, 
                zoidfs::zoidfs_op_hint_t * op_hint)
  {
       /* create the empty wrapper */
       CBSMWrapper * cbsm = CBSMWrapper::createCBSMWrapper(cb);

       /* Sets up the handler for the RPC State Machine */
       /* Should be changed to RPC KEY */
       rpc::RPCClientHandle rpc_handle = client_->rpcConnect(ZOIDFS_SETATTR_RPC.c_str(), addr_);
       boost::shared_ptr<RPCCommSetAttr> comm;
       comm.reset(new RPCCommSetAttr (smm_, rpc_handle, poll_));

       /* create the state machine */
       sm::SMClientSharedPtr sm(new iofwdclient::clientsm::SetAttrClientSM(*smm_, poll_, comm,
                                cbsm->getWCB(), ret, handle, sattr, attr, op_hint));
      
       /* add the sm to the cb wrapper */ 
       cbsm->set(sm);

       /* execute the sm */
       sm->execute();
  }
  void CBClient::cbreadlink(const IOFWDClientCB & cb,
               int * ret,
               const zoidfs::zoidfs_handle_t * handle,  
               char * buffer,
               size_t buffer_length,
               zoidfs::zoidfs_op_hint_t * op_hint)
  {
       /* create the empty wrapper */
       CBSMWrapper * cbsm = CBSMWrapper::createCBSMWrapper(cb);

       /* Sets up the handler for the RPC State Machine */
       /* Should be changed to RPC KEY */
       rpc::RPCClientHandle rpc_handle = client_->rpcConnect(ZOIDFS_READLINK_RPC.c_str(), addr_);
       boost::shared_ptr<RPCCommReadLink> comm;
       comm.reset(new RPCCommReadLink (smm_, rpc_handle, poll_));

       /* create the state machine */
       sm::SMClientSharedPtr sm(new iofwdclient::clientsm::ReadLinkClientSM(*smm_, poll_, comm,
                                cbsm->getWCB(), ret, handle, buffer, buffer_length, op_hint));
      
       /* add the sm to the cb wrapper */ 
       cbsm->set(sm);

       /* execute the sm */
       sm->execute();


  }

   
   void CBClient::cbremove(const IOFWDClientCB & cb,
                          int * ret,
                          const zoidfs::zoidfs_handle_t * parent_handle,
                          const char * component_name, const char * full_path,
                          zoidfs::zoidfs_cache_hint_t * parent_hint,
                          zoidfs::zoidfs_op_hint_t * op_hint)
  {
       /* create the empty wrapper */
       CBSMWrapper * cbsm = CBSMWrapper::createCBSMWrapper(cb);

       /* Sets up the handler for the RPC State Machine */
       /* Should be changed to RPC KEY */
       rpc::RPCClientHandle rpc_handle = client_->rpcConnect(ZOIDFS_REMOVE_RPC.c_str(), addr_);
       boost::shared_ptr<RPCCommRemove> comm;
       comm.reset(new RPCCommRemove (smm_, rpc_handle, poll_));
       
       /* create the state machine */
       sm::SMClientSharedPtr sm(new iofwdclient::clientsm::RemoveClientSM(*smm_, poll_, comm, 
                                cbsm->getWCB(), ret, parent_handle, component_name, full_path,
                                parent_hint, op_hint));
      
       /* add the sm to the cb wrapper */ 
       cbsm->set(sm);

       /* execute the sm */
       sm->execute();   
  } 

   
  void CBClient::cbrename(const IOFWDClientCB & cb,
                int * ret,
                const zoidfs::zoidfs_handle_t * from_parent_handle,
                const char * from_component_name,
                const char * from_full_path,
                const zoidfs::zoidfs_handle_t * to_parent_handle,
                const char * to_component_name,
                const char * to_full_path,
                zoidfs::zoidfs_cache_hint_t * from_parent_hint,
                zoidfs::zoidfs_cache_hint_t * to_parent_hint,
                zoidfs::zoidfs_op_hint_t * op_hint) 
  {
       /* create the empty wrapper */
       CBSMWrapper * cbsm = CBSMWrapper::createCBSMWrapper(cb);

       /* Sets up the handler for the RPC State Machine */
       /* Should be changed to RPC KEY */
       rpc::RPCClientHandle rpc_handle = client_->rpcConnect(ZOIDFS_RENAME_RPC.c_str(), addr_);
       boost::shared_ptr<RPCCommRename> comm;
       comm.reset(new RPCCommRename (smm_, rpc_handle, poll_));
       
       /* create the state machine */
       sm::SMClientSharedPtr sm(new iofwdclient::clientsm::RenameClientSM(*smm_, poll_, comm, 
                                cbsm->getWCB(), ret, from_parent_handle, from_component_name,
                                from_full_path, to_parent_handle, to_component_name, to_full_path,
                                from_parent_hint, to_parent_hint, op_hint));
      
       /* add the sm to the cb wrapper */ 
       cbsm->set(sm);

       /* execute the sm */
       sm->execute();   
  }



  void CBClient::cbsymlink(const IOFWDClientCB & cb,
                          int * ret,
                          const zoidfs::zoidfs_handle_t * from_parent_handle,
                          const char * from_component_name,
                          const char * from_full_path,
                          const zoidfs::zoidfs_handle_t * to_parent_handle,
                          const char * to_component_name,
                          const char * to_full_path,
                          const zoidfs::zoidfs_sattr_t * sattr,
                          zoidfs::zoidfs_cache_hint_t * from_parent_hint,
                          zoidfs::zoidfs_cache_hint_t * to_parent_hint,
                          zoidfs::zoidfs_op_hint_t * op_hint)
  {
       /* create the empty wrapper */
       CBSMWrapper * cbsm = CBSMWrapper::createCBSMWrapper(cb);

       /* Sets up the handler for the RPC State Machine */
       /* Should be changed to RPC KEY */
       rpc::RPCClientHandle rpc_handle = client_->rpcConnect(ZOIDFS_SYMLINK_RPC.c_str(), addr_);
       boost::shared_ptr<RPCCommSymLink> comm;
       comm.reset(new RPCCommSymLink (smm_, rpc_handle, poll_));
       
       /* create the state machine */
       sm::SMClientSharedPtr sm(new iofwdclient::clientsm::SymLinkClientSM(*smm_, poll_, comm, 
                                cbsm->getWCB(), ret, from_parent_handle, from_component_name,
                                from_full_path, to_parent_handle, to_component_name, to_full_path, sattr,
                                from_parent_hint, to_parent_hint, op_hint));
      
       /* add the sm to the cb wrapper */ 
       cbsm->set(sm);

       /* execute the sm */
       sm->execute();   
  }

  void CBClient::cbreaddir(const IOFWDClientCB & cb,
                          int * ret,
                          const zoidfs::zoidfs_handle_t * parent_handle,
                          zoidfs::zoidfs_dirent_cookie_t cookie, size_t * entry_count_,
                          zoidfs::zoidfs_dirent_t * entries, uint32_t flags,
                          zoidfs::zoidfs_cache_hint_t * parent_hint,
                          zoidfs::zoidfs_op_hint_t * op_hint) 
  {

       /* create the empty wrapper */
       CBSMWrapper * cbsm = CBSMWrapper::createCBSMWrapper(cb);

       /* Sets up the handler for the RPC State Machine */
       /* Should be changed to RPC KEY */
       rpc::RPCClientHandle rpc_handle = client_->rpcConnect(ZOIDFS_READDIR_RPC.c_str(), addr_);
       boost::shared_ptr<RPCCommReadDir> comm;
       comm.reset(new RPCCommReadDir (smm_, rpc_handle, poll_));
       
       /* create the state machine */
       sm::SMClientSharedPtr sm(new iofwdclient::clientsm::ReadDirClientSM(*smm_, poll_, comm, 
                                cbsm->getWCB(), ret, parent_handle, cookie, entry_count_,
                                entries, flags, parent_hint, op_hint));
      
       /* add the sm to the cb wrapper */ 
       cbsm->set(sm);

       /* execute the sm */
       sm->execute();   


  }

   void CBClient::cbmkdir(const IOFWDClientCB & cb,
                          int * ret,
                          const zoidfs::zoidfs_handle_t * parent_handle,
                          const char * component_name, const char * full_path,
                          const zoidfs::zoidfs_sattr_t * sattr,
                          zoidfs::zoidfs_cache_hint_t * parent_hint,
                          zoidfs::zoidfs_op_hint_t * op_hint)
   {
       /* create the empty wrapper */
       CBSMWrapper * cbsm = CBSMWrapper::createCBSMWrapper(cb);

       /* Sets up the handler for the RPC State Machine */
       /* Should be changed to RPC KEY */
       rpc::RPCClientHandle rpc_handle = client_->rpcConnect(ZOIDFS_MKDIR_RPC.c_str(), addr_);
       boost::shared_ptr<RPCCommMkdir> comm;
       comm.reset(new RPCCommMkdir (smm_, rpc_handle, poll_));
       
       /* create the state machine */
       sm::SMClientSharedPtr sm(new iofwdclient::clientsm::MkdirClientSM(*smm_, poll_, comm, 
                                cbsm->getWCB(), ret, parent_handle, component_name, full_path, sattr,
                                parent_hint, op_hint));
      
       /* add the sm to the cb wrapper */ 
       cbsm->set(sm);

       /* execute the sm */
       sm->execute();      
   }


   void CBClient::cbcreate(const IOFWDClientCB & cb,
                          int * ret,
                          const zoidfs::zoidfs_handle_t *parent_handle,
                          const char *component_name, 
                          const char *full_path,
                          const zoidfs::zoidfs_sattr_t *sattr, 
                          zoidfs::zoidfs_handle_t *handle,
                          int *created,
                          zoidfs::zoidfs_op_hint_t * op_hint)
   {
       /* create the empty wrapper */
       CBSMWrapper * cbsm = CBSMWrapper::createCBSMWrapper(cb);

       /* Sets up the handler for the RPC State Machine */
       /* Should be changed to RPC KEY */
       rpc::RPCClientHandle rpc_handle = client_->rpcConnect(ZOIDFS_CREATE_RPC.c_str(), addr_);
       boost::shared_ptr<RPCCommClientSMCreate> comm;
       comm.reset(new RPCCommClientSMCreate (smm_, rpc_handle, poll_));
       
       /* create the state machine */
       sm::SMClientSharedPtr sm(new iofwdclient::clientsm::CreateClientSM(*smm_, poll_, comm, 
                                cbsm->getWCB(), ret, parent_handle, component_name, full_path,
                                sattr, handle, created, op_hint));
      
       /* add the sm to the cb wrapper */ 
       cbsm->set(sm);

       /* execute the sm */
       sm->execute();      
   }

   void CBClient::cblookup(const IOFWDClientCB & cb,
                          int * ret,
                          const zoidfs::zoidfs_handle_t *parent_handle,
                          const char *component_name, 
                          const char *full_path,
                          zoidfs::zoidfs_handle_t *handle,
                          zoidfs::zoidfs_op_hint_t * op_hint)
   {
       /* create the empty wrapper */
       CBSMWrapper * cbsm = CBSMWrapper::createCBSMWrapper(cb);

       /* Sets up the handler for the RPC State Machine */
       /* Should be changed to RPC KEY */
       rpc::RPCClientHandle rpc_handle = client_->rpcConnect(ZOIDFS_LOOKUP_RPC.c_str(), addr_);
       boost::shared_ptr<RPCCommClientSMLookup> comm;
       comm.reset(new RPCCommClientSMLookup (smm_, rpc_handle, poll_));
       
       /* create the state machine */
       sm::SMClientSharedPtr sm(new iofwdclient::clientsm::LookupClientSM(*smm_, poll_, comm, 
                                cbsm->getWCB(), ret, parent_handle, component_name, full_path,
                                handle, op_hint));
      
       /* add the sm to the cb wrapper */ 
       cbsm->set(sm);

       /* execute the sm */
       sm->execute();      
   }

   void CBClient::cbwrite(const IOFWDClientCB & cb,
              int * ret,
              const zoidfs::zoidfs_handle_t *handle, size_t mem_count,
              const void *mem_starts[], const size_t mem_sizes[],
              size_t file_count, const zoidfs::zoidfs_file_ofs_t file_starts[],
              zoidfs::zoidfs_file_ofs_t file_sizes[],
              zoidfs::zoidfs_op_hint_t * op_hint)
   {
       /* create the empty wrapper */
       CBSMWrapper * cbsm = CBSMWrapper::createCBSMWrapper(cb);

       /* Sets up the handler for the RPC State Machine */
       /* Should be changed to RPC KEY */
       rpc::RPCClientHandle rpc_handle = client_->rpcConnect(ZOIDFS_WRITE_RPC.c_str(), addr_);
       boost::shared_ptr<RPCCommClientSMWrite> comm;
       comm.reset(new RPCCommClientSMWrite (smm_, rpc_handle, poll_));
       
       /* create the state machine */
       sm::SMClientSharedPtr sm(new iofwdclient::clientsm::WriteClientSM(*smm_, poll_, comm, 
                                cbsm->getWCB(), ret, handle, mem_count, mem_starts, mem_sizes,
                                file_count, file_starts, file_sizes, op_hint));
      
       /* add the sm to the cb wrapper */ 
       cbsm->set(sm);

       /* execute the sm */
       sm->execute();           
   }
   
   void CBClient::cbread(const IOFWDClientCB & cb,
                        int * ret,
                        const zoidfs::zoidfs_handle_t *handle, size_t mem_count,
                        void *mem_starts[], const size_t mem_sizes[],
                        size_t file_count, 
                        const zoidfs::zoidfs_file_ofs_t file_starts[],
                        zoidfs::zoidfs_file_ofs_t file_sizes[],
                        zoidfs::zoidfs_op_hint_t * op_hint)
   {
       /* create the empty wrapper */
       CBSMWrapper * cbsm = CBSMWrapper::createCBSMWrapper(cb);

       /* Sets up the handler for the RPC State Machine */
       /* Should be changed to RPC KEY */
       rpc::RPCClientHandle rpc_handle = client_->rpcConnect(ZOIDFS_READ_RPC.c_str(), addr_);
       boost::shared_ptr<RPCCommClientSMRead> comm;
       comm.reset(new RPCCommClientSMRead (smm_, rpc_handle, poll_));
       
       /* create the state machine */
       sm::SMClientSharedPtr sm(new iofwdclient::clientsm::ReadClientSM(*smm_, poll_, comm, 
                                cbsm->getWCB(), ret, handle, mem_count, mem_starts, mem_sizes,
                                file_count, file_starts, file_sizes, op_hint));
      
       /* add the sm to the cb wrapper */ 
       cbsm->set(sm);

       /* execute the sm */
       sm->execute();           
   }

   //========================================================================
}
