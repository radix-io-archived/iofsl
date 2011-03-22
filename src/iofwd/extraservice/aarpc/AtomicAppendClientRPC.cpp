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
            net_(netservice_->getNet())
        {
            /* if in master mode, setup the client to contact the master */
            if(strcmp(iofwd::extraservice::AtomicAppendServerRPC::aarpc_master_addr_.c_str(),
                        "") != 0)
            {
                master_mode_ = true;
                iofwdevent::SingleCompletion block;
            
                /* get the address */
                net_->lookup(iofwd::extraservice::AtomicAppendServerRPC::aarpc_master_addr_.c_str(),
                        &addr_, block);

                /* wait for the lookup to complete */
                block.wait();
            }
            /* else, setup the communicator */
            else
            {
                master_mode_ = false;
                comm_ = netservice_->getServerComm();
                comm_size_ = comm_->size();
            }
        }

        AtomicAppendClientRPC::~AtomicAppendClientRPC()
        {
        }
    }
}
