#include "iofwdclient/clientsm/LinkClientSM.hh"

namespace iofwdclient
{
    namespace clientsm
    {

      LinkClientSM::~LinkClientSM()
      {
      }

      void LinkClientSM::init(iofwdevent::CBException e)
      {
          e.check();
          setNextMethod(&LinkClientSM::postRPCServerSM);
      }

      void LinkClientSM::postRPCServerSM(iofwdevent::CBException e)
      {
          e.check();

          /* Runs the RPC Client State Machine */
          comm_->connect(in_, out_, slots_[BASE_SLOT]);

          /* Set up slot wait for completion */
          slots_.wait(BASE_SLOT, &LinkClientSM::waitRPCServerSM);
      }

      void LinkClientSM::waitRPCServerSM(iofwdevent::CBException e)
      {
          e.check();
          *ret_ = out_.returnCode;
          *from_parent_hint_ = out_.from_parent_hint;
          *to_parent_hint_ = out_.to_parent_hint;
          cb_(zoidfs::ZFS_COMP_DONE, e);
      }

      void LinkClientSM::postSMErrorState(iofwdevent::CBException e)
      {
          e.check();
          cb_(zoidfs::ZFS_COMP_ERROR, e);
      }

    }
}
