#include "iofwd/extraservice/aarpc/AtomicAppendClientRPC.hh"

#include "iofwd/service/Service.hh"

SERVICE_REGISTER(iofwd::extraservice::AtomicAppendClientRPC, aarpcclient);

namespace iofwd
{
    namespace extraservice
    {
        AtomicAppendClientRPC::AtomicAppendClientRPC(service::ServiceManager & m) :
            ExtraService(m),
            man_(iofwd::service::ServiceManager::instance()),
            netservice_(man_.loadService<iofwd::Net>("net")),
            rpcclient_(man_.loadService<iofwd::RPCClient>("rpcclient")),
            net_(netservice_->getNet()),
            comm_(netservice_->getServerComm()),
            comm_size_(comm_->size())
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
