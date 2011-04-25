#ifndef IOFWDCLIENT_SM_RPCCOMMWRITESM
#define IOFWDCLIENT_SM_RPCCOMMWRITESM

#include "iofwdutil/tools.hh"

#include "iofwdclient/clientsm/RPCClientWrite.hh"

#include "rpc/RPCKey.hh"
#include "sm/SMManager.hh"
#include "iofwdclient/IOFWDClientCB.hh"

#include "sm/SMClient.hh"

namespace iofwdclient
{
    using namespace streamwrappers;

namespace clientsm
{
   template< typename INTYPE, typename OUTTYPE >
   class RPCCommWriteSM
   {
      public:

         RPCCommWriteSM(boost::shared_ptr<sm::SMManager> smm,
                        rpc::RPCClientHandle rpc_handle,
                        bool poll) :
            poll_(poll),
            smm_(smm),
            rpc_handle_(rpc_handle)
         {
         }

         void connect(const INTYPE & in_, OUTTYPE & out_, 
                      const iofwdevent::CBType & cb_)
         {
            server_sm_ .reset(new RPCClientWrite<INTYPE,OUTTYPE>(*smm_, poll_, cb_,
                                                            rpc_handle_, in_, 
                                                            out_));
	          server_sm_->execute();
         }

      protected:
         bool poll_;
         boost::shared_ptr<sm::SMManager> smm_;
         sm::SMClientSharedPtr server_sm_;
         rpc::RPCClientHandle rpc_handle_;
   };
}
}
#endif
