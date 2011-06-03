#include "iofwdclient/clientsm/SetAttrClientSM.hh"
//#include "iofwdclient/FakeBlocker.hh"

#include "zoidfs/zoidfs-async.h"
#include "zoidfs/zoidfs-rpc.h"

#include <cstdio>

namespace iofwdclient
{
    namespace clientsm
    {

      SetAttrClientSM::~SetAttrClientSM()
      {
      }

      void SetAttrClientSM::init(iofwdevent::CBException e)
      {
          e.check();
          setNextMethod(&SetAttrClientSM::postRPCServerSM);
      }

      void SetAttrClientSM::postRPCServerSM(iofwdevent::CBException e)
      {
          e.check();

          /* Runs the RPC Client State Machine */
          comm_->connect(in_, out_, slots_[BASE_SLOT]);

          /* Set up slot wait for completion */
          slots_.wait(BASE_SLOT, &SetAttrClientSM::waitRPCServerSM);
      }

      void SetAttrClientSM::waitRPCServerSM(iofwdevent::CBException e)
      {
          e.check();
          *ret_ = out_.returnCode;
          *attr_ = out_.attr_enc;
          cb_(zoidfs::ZFS_COMP_DONE, e);
      }

      void SetAttrClientSM::postSMErrorState(iofwdevent::CBException e)
      {
          e.check();
          cb_(zoidfs::ZFS_COMP_ERROR, e);
      }

    
    }
}
