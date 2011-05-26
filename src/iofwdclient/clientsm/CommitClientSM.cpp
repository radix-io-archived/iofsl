#include "iofwdclient/clientsm/CommitClientSM.hh"

namespace iofwdclient
{
    namespace clientsm
    {

      CommitClientSM::~CommitClientSM()
      {
      }

      void CommitClientSM::init(iofwdevent::CBException e)
      {
          e.check();
          setNextMethod(&CommitClientSM::postRPCServerSM);
      }

      void CommitClientSM::postRPCServerSM(iofwdevent::CBException e)
      {
          e.check();

          /* Runs the RPC Client State Machine */
          comm_->connect(in_, out_, slots_[BASE_SLOT]);

          /* Set up slot wait for completion */
          slots_.wait(BASE_SLOT, &CommitClientSM::waitRPCServerSM);
      }

      void CommitClientSM::waitRPCServerSM(iofwdevent::CBException e)
      {
          e.check();
          *ret_ = out_.returnCode;
          cb_(zoidfs::ZFS_COMP_DONE, e);
      }

      void CommitClientSM::postSMErrorState(iofwdevent::CBException e)
      {
          e.check();
          cb_(zoidfs::ZFS_COMP_ERROR, e);
      }

    }
}
