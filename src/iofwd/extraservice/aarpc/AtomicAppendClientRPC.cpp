#include "iofwd/extraservice/aarpc/AtomicAppendClientRPC.hh"

namespace iofwd
{
    namespace extraservice
    {
        AtomicAppendClientRPC::AtomicAppendClientRPC() :
            man_(iofwd::service::ServiceManager::instance()),
            netservice_(man_.loadService<iofwd::Net>("net")),
            rpcclient_(man_.loadService<iofwd::RPCClient>("rpcclient"))
        {
            iofwdevent::SingleCompletion block;
            
            /* get the address */
            net::Net * net = netservice_->getNet();
            net->lookup(iofwd::extraservice::AtomicAppendServerRPC::aarpc_master_addr_.c_str(),
                    &addr_, block);

            /* wait for the lookup to complete */
            block.wait();
        }

        AtomicAppendClientRPC::~AtomicAppendClientRPC()
        {
        }
    }
}
