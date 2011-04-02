#include <cstdio>
#include "iofwdclient/CBClient.hh"

#include "iofwdclient/CommStream.hh"

#include "iofwdutil/IOFWDLog.hh"
#include "iofwdutil/tools.hh"

#include "iofwdclient/clientsm/GetAttrClientSM.hh"

#include "iofwdclient/clientsm/LookupClientSM.hh"
#include "iofwdclient/streamwrappers/LookupStreams.hh"
#include "iofwdclient/clientsm/WriteClientSM.hh"
#include "iofwdclient/streamwrappers/WriteStreams.hh"

#include "iofwdclient/clientsm/RPCCommClientSM.hh"
#include "iofwdclient/clientsm/RPCCommWriteSM.hh"

using namespace zoidfs;

namespace iofwdclient
{
   typedef iofwdclient::clientsm::RPCCommClientSM<LookupInStream, LookupOutStream> RPCCommClientSMLookup;
   typedef iofwdclient::clientsm::RPCCommWriteSM<WriteInStream, WriteOutStream> RPCCommClientSMWrite;
   //========================================================================
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
//    @TODO: This needs to be changed, possibly use the CommStream class?
//   void CBClient::setRPCMode ( boost::shared_ptr<iofwd::RPCClient> rpcclient,
//                              net::AddressPtr addr)
//   {
//      client_ = rpcclient;
//      addr_ = addr;
//   }

   void CBClient::cbWrapper(CBSMWrapper * cbsm,
           zoidfs::zoidfs_comp_mask_t mask,
           const iofwdevent::CBException & cbexception)
   {
       fprintf(stderr, "%s:%i callback wrapper invoked, mask = %i\n",
               __func__, __LINE__, mask);

       /* call the original cb */
       cbsm->call(mask, cbexception);

       /* delete the wrapper... trigger ref count dec on SM */
       delete cbsm;
   }

   void CBClient::cbgetattr(const IOFWDClientCB & cb,
         int * ret,
         const zoidfs_handle_t * handle,
         zoidfs_attr_t * attr,
         zoidfs_op_hint_t * op_hint)
   {
       /* create the empty wrapper */
       CBSMWrapper * cbsm = CBSMWrapper::createCBSMWrapper(cb);

       /* create the state machine */
       iofwdclient::clientsm::GetAttrClientSM * sm =
           new iofwdclient::clientsm::GetAttrClientSM(*smm_, poll_,
                   cbsm->getWCB(), addr_, ret, handle, attr, op_hint);
      
       /* add the sm to the cb wrapper */ 
       cbsm->set(sm);

       /* execute the sm */
       sm->execute();
   }

   int CBClient::cblookup (   const IOFWDClientCB & cb,
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
       iofwdclient::clientsm::LookupClientSM * sm =
           new iofwdclient::clientsm::LookupClientSM(*smm_, poll_, comm, 
                   cbsm->getWCB(), ret, parent_handle, component_name, full_path,
                   handle, op_hint);
      
       /* add the sm to the cb wrapper */ 
       cbsm->set(sm);

       /* execute the sm */
       sm->execute();      
   }

   int CBClient::cbwrite(const IOFWDClientCB & cb,
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
       iofwdclient::clientsm::WriteClientSM * sm =
           new iofwdclient::clientsm::WriteClientSM(*smm_, poll_, comm, 
                   cbsm->getWCB(), ret, handle, mem_count, mem_starts, mem_sizes,
                   file_count, file_starts, file_sizes, op_hint);
      
       /* add the sm to the cb wrapper */ 
       cbsm->set(sm);

       /* execute the sm */
       sm->execute();           
   }
   

   //========================================================================
}
