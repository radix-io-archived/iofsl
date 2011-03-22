#include "iofwdclient/CBClient.hh"

#include "iofwdclient/CommStream.hh"

#include "iofwdutil/IOFWDLog.hh"
#include "iofwdutil/tools.hh"

#include "iofwdclient/clientsm/GetAttrClientSM.hh"
#include "iofwdclient/clientsm/LookupClientSM.hh"
using namespace zoidfs;

namespace iofwdclient
{
   //========================================================================

   CBClient::CBClient (iofwdutil::IOFWDLogSource & log,
         CommStream & net, bool poll)
      : log_ (log),
        net_ (net),
        poll_(poll),
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
                   cbsm->getWCB(), ret, handle, attr, op_hint);
      
       /* add the sm to the cb wrapper */ 
       cbsm->set(sm);

       /* execute the sm */
       sm->execute();
   }

   int CBClient::cblookup (  const IOFWDClientCB & cb,
                              int * ret,
                              const zoidfs::zoidfs_handle_t *parent_handle,
                              const char *component_name, 
                              const char *full_path,
                              zoidfs::zoidfs_handle_t *handle,  
                              zoidfs::zoidfs_op_hint_t * op_hint)
   {
       /* create the empty wrapper */
       CBSMWrapper * cbsm = CBSMWrapper::createCBSMWrapper(cb);

       /* create the state machine */
       iofwdclient::clientsm::LookupClientSM * sm =
           new iofwdclient::clientsm::LookupClientSM(*smm_, poll_,
                   cbsm->getWCB(), ret, parent_handle, component_name, full_path,
                   handle, op_hint);
      
       /* add the sm to the cb wrapper */ 
       cbsm->set(sm);

       /* execute the sm */
       sm->execute();      
   }

   //========================================================================
}
