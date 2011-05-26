#include "iofwdclient/clientsm/GetAttrClientSM.hh"
//#include "iofwdclient/FakeBlocker.hh"

#include "zoidfs/zoidfs-async.h"
#include "zoidfs/zoidfs-rpc.h"

#include <cstdio>

namespace iofwdclient
{
    namespace clientsm
    {

      GetAttrClientSM::~GetAttrClientSM()
      {
      }

      void GetAttrClientSM::init(iofwdevent::CBException e)
      {
          e.check();
          setNextMethod(&GetAttrClientSM::postRPCServerSM);
      }

      void GetAttrClientSM::postRPCServerSM(iofwdevent::CBException e)
      {
          e.check();

          /* Runs the RPC Client State Machine */
          comm_->connect(in_, out_, slots_[BASE_SLOT]);

          /* Set up slot wait for completion */
          slots_.wait(BASE_SLOT, &GetAttrClientSM::waitRPCServerSM);
      }

      void GetAttrClientSM::waitRPCServerSM(iofwdevent::CBException e)
      {
          e.check();
          *ret_ = out_.returnCode;
          *attr_ = out_.attr_enc;
          cb_(zoidfs::ZFS_COMP_DONE, e);
      }

      void GetAttrClientSM::postSMErrorState(iofwdevent::CBException e)
      {
          e.check();
          cb_(zoidfs::ZFS_COMP_ERROR, e);
      }

    
    }
}
