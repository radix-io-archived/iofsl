#include "iofwdclient/CBClient.hh"

#include "iofwdutil/IOFWDLog.hh"
#include "iofwdutil/tools.hh"

#include "iofwdclient/clientsm/GetAttrClientSM.hh"

using namespace zoidfs;

namespace iofwdclient
{
   //========================================================================

   CBClient::CBClient (bool poll)
      : log_ (iofwdutil::IOFWDLog::getSource ()),
      smm_(new sm::SMManager(poll)),
      poll_(poll)
   {
   }

   CBClient::~CBClient ()
   {
   }

   void CBClient::cbgetattr(const IOFWDClientCB & cb,
         int * ret,
         const zoidfs_handle_t * handle,
         zoidfs_attr_t * attr,
         zoidfs_op_hint_t * op_hint)
   {
       /* create the state machine */
       iofwdclient::clientsm::GetAttrClientSM * sm =
           new iofwdclient::clientsm::GetAttrClientSM(*smm_, poll_, cb, ret,
                   handle, attr, op_hint);

       sm->execute();

       /* submit the state machine to the SMManager */
       //smm_->schedule(sm);

       /* TODO where are we tracking the state machines */
       temp_smclient_queue_.push(sm);
   }

   //========================================================================
}
