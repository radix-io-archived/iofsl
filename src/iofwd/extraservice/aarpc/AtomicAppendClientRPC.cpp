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
            comm_size_(1),
            offset_init_func_(&AtomicAppendClientRPC::aarpcOffsetLocalInitializer)
        {
            /* else use the local fast path */
            if(strcmp(iofwd::extraservice::AtomicAppendServerRPC::aarpc_master_addr_.c_str(),
                        "localhost") == 0)
            {
                master_mode_ = false;
                distributed_mode_ = false;
            }
            /* if in master mode, setup the client to contact the master */
            else if(strcmp(iofwd::extraservice::AtomicAppendServerRPC::aarpc_master_addr_.c_str(),
                        "") != 0)
            {
                master_mode_ = true;
                distributed_mode_ = false;
                iofwdevent::SingleCompletion block;
            
                /* get the address */
                net_->lookup(iofwd::extraservice::AtomicAppendServerRPC::aarpc_master_addr_.c_str(),
                        &addr_, block);

                /* wait for the lookup to complete */
                block.wait();
            }
            else
            {
                master_mode_ = false;
                distributed_mode_ = true;
                comm_ = netservice_->getServerComm();
                comm_size_ = comm_->size();
            }
        }

        AtomicAppendClientRPC::~AtomicAppendClientRPC()
        {
        }
    }
}
