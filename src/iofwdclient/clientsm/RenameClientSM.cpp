#include "iofwdclient/clientsm/RenameClientSM.hh"

namespace iofwdclient
{
    namespace clientsm
    {

      RenameClientSM::~RenameClientSM()
      {
      }

      void RenameClientSM::init(iofwdevent::CBException e)
      {
          e.check();
          setNextMethod(&RenameClientSM::postRPCServerSM);
      }

      void RenameClientSM::postRPCServerSM(iofwdevent::CBException e)
      {
          e.check();

          /* Runs the RPC Client State Machine */
          comm_->connect(in_, out_, slots_[BASE_SLOT]);

          /* Set up slot wait for completion */
          slots_.wait(BASE_SLOT, &RenameClientSM::waitRPCServerSM);
      }

      void RenameClientSM::waitRPCServerSM(iofwdevent::CBException e)
      {
          e.check();
          *ret_ = out_.returnCode;
          *from_parent_hint_ = out_.from_parent_hint;
          *to_parent_hint_ = out_.to_parent_hint;
          cb_(zoidfs::ZFS_COMP_DONE, e);
      }

      void RenameClientSM::postSMErrorState(iofwdevent::CBException e)
      {
          e.check();
          cb_(zoidfs::ZFS_COMP_ERROR, e);
      }

    }
}
