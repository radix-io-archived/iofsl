#include "iofwdclient/clientsm/ReadDirClientSM.hh"

namespace iofwdclient
{
    namespace clientsm
    {

      ReadDirClientSM::~ReadDirClientSM()
      {
      }

      void ReadDirClientSM::init(iofwdevent::CBException e)
      {
          e.check();
          setNextMethod(&ReadDirClientSM::postRPCServerSM);
      }

      void ReadDirClientSM::postRPCServerSM(iofwdevent::CBException e)
      {
          e.check();

          /* Runs the RPC Client State Machine */
          comm_->connect(in_, out_, slots_[BASE_SLOT]);

          /* Set up slot wait for completion */
          slots_.wait(BASE_SLOT, &ReadDirClientSM::waitRPCServerSM);
      }

      void ReadDirClientSM::waitRPCServerSM(iofwdevent::CBException e)
      {
          e.check();
          *ret_ = out_.returnCode;
          *parent_hint_ = out_.cache;
          cb_(zoidfs::ZFS_COMP_DONE, e);
      }

      void ReadDirClientSM::postSMErrorState(iofwdevent::CBException e)
      {
          e.check();
          cb_(zoidfs::ZFS_COMP_ERROR, e);
      }

    }
}
