#ifndef IOFWD_EXTRASERVICE_AARPC_ATOMICAPPENDCLIENTRPC_HH
#define IOFWD_EXTRASERVICE_AARPC_ATOMICAPPENDCLIENTRPC_HH

#include "iofwd/service/ServiceManager.hh"
#include "iofwd/RPCClient.hh"
#include "iofwd/Net.hh"
#include "iofwd/IofwdLinkHelper.hh"
#include "iofwd/service/Service.hh"
#include "iofwdutil/ZException.hh"
#include "iofwdevent/SingleCompletion.hh"
#include "net/Net.hh"

#include "rpc/RPCInfo.hh"
#include "rpc/RPCEncoder.hh"

#include <string>
#include <unistd.h>

#include "zoidfs/zoidfs.h"
#include "zoidfs/util/zoidfs-xdr.hh"

#include "iofwd/extraservice/aarpc/AtomicAppendServerRPC.hh"

namespace iofwd
{
    namespace extraservice
    {
        class AtomicAppendClientRPC
        {
            public:
                AtomicAppendClientRPC() :
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

                ~AtomicAppendClientRPC()
                {
                }

                void getNextOffset(zoidfs::zoidfs_handle_t & handle,
                        zoidfs::zoidfs_file_size_t incsize,
                        zoidfs::zoidfs_file_ofs_t & offset,
                        uint64_t & retcode)
                {
                    getNextOffsetImpl(
                            rpcclient_->rpcConnect("aarpc.getnextoffset", addr_),
                            handle, incsize, offset, retcode);
                }

                void createOffset(zoidfs::zoidfs_handle_t & handle,
                        zoidfs::zoidfs_file_ofs_t & offset,
                        uint64_t & retcode)
                {
                    createOffsetImpl(
                            rpcclient_->rpcConnect("aarpc.createoffset", addr_),
                            handle, offset, retcode);
                }

                void deleteOffset(zoidfs::zoidfs_handle_t & handle,
                        uint64_t & retcode)
                {
                    deleteOffsetImpl(
                            rpcclient_->rpcConnect("aarpc.deleteoffset", addr_),
                            handle, retcode);
                }

            protected:
                void getNextOffsetImpl(rpc::RPCClientHandle h,
                        zoidfs::zoidfs_handle_t & handle,
                        zoidfs::zoidfs_file_size_t incsize,
                        zoidfs::zoidfs_file_ofs_t & offset,
                        uint64_t & retcode);
                
                void createOffsetImpl(rpc::RPCClientHandle h,
                        zoidfs::zoidfs_handle_t & handle,
                        zoidfs::zoidfs_file_ofs_t offset,
                        uint64_t & retcode);
                
                void deleteOffsetImpl(rpc::RPCClientHandle h,
                        zoidfs::zoidfs_handle_t & handle,
                        uint64_t & retcode);

                iofwd::service::ServiceManager & man_;
                boost::shared_ptr<iofwd::Net> netservice_;
                boost::shared_ptr<iofwd::RPCClient> rpcclient_;
                net::AddressPtr addr_;
        };
    }
}

#endif
