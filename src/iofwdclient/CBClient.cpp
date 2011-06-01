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
       rpc::RPCClientHandle rpc_handle = client_->rpcConnect(ZOIDFS_CREATE_RPC.c_str(), addr_);
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
