#ifndef IOFWDCLIENT_CLIENTSM_RPCSERVERSM_HH
#define IOFWDCLIENT_CLIENTSM_RPCSERVERSM_HH

#include "sm/SMManager.hh"
#include "sm/SMClient.hh"
#include "sm/SimpleSM.hh"
#include "sm/SimpleSlots.hh"

#include "iofwdutil/tools.hh"

#include "iofwdclient/IOFWDClientCB.hh"
#include "iofwdclient/clientsm/RPCServerHelper.hh"

#include "iofwdclient/streamwrappers/ZoidFSStreamWrappers.hh"

#include "iofwd/RPCClient.hh"
#include "iofwd/service/ServiceManager.hh"

#include "iofwdevent/ZeroCopyInputStream.hh"
#include "iofwdevent/ZeroCopyOutputStream.hh"

#include "zoidfs/zoidfs.h"
#include "zoidfs/zoidfs-rpc.h"

#include "rpc/RPCEncoder.hh"

#include <cstdio>

using namespace iofwdclient;
using namespace iofwdclient::streamwrappers;
using namespace encoder;
using namespace encoder::xdr;

namespace iofwdclient
{
    namespace clientsm
    {

template< typename InStream, typename OutStream >
class RPCServerSM :
    public sm::SimpleSM< iofwdclient::clientsm::RPCServerSM <InStream, OutStream> >
{
    public:
        RPCServerSM(sm::SMManager & smm,
                bool poll,
                const iofwdevent::CBType & cb,
                std::string rpc_func,
                const InStream & in,
                OutStream & out) :
            sm::SimpleSM< iofwdclient::clientsm::RPCServerSM <InStream,OutStream> >(smm, poll),
            slots_(*this),
            cb_(cb),
            rpc_func_(rpc_func),
            rpc_client_(iofwd::service::ServiceManager::instance().loadService<iofwd::RPCClient>("rpcclient")),
            e_(in),
            d_(out)
        {
            fprintf(stderr, "%s:%i\n", __func__, __LINE__);
        }

        ~RPCServerSM()
        {
        }

        void init(iofwdevent::CBException e)
        {
            e.check();
            setNextMethod(&RPCServerSM<InStream,OutStream>::postSetupConnection);
        }

        void postSetupConnection(iofwdevent::CBException e)
        {
            e.check();

            /* get the max size */
            e_.net_data_size_ = rpc::getRPCEncodedSize(InStream()).getMaxSize();

            /* TODO how the heck do we get / set the address? */

            /* get a handle for this RPC */
            rpc_handle_ = rpc_client_->rpcConnect(rpc_func_.c_str(), addr_);
            e_.zero_copy_stream_.reset((rpc_handle_->getOut()));

            /* setup the write stream */
            e_.zero_copy_stream_->write(&e_.data_ptr_, &e_.data_size_, slots_[BASE_SLOT],
                    e_.net_data_size_);

            slots_.wait(BASE_SLOT,
                    &RPCServerSM<InStream,OutStream>::waitSetupConnection);
        }

        void waitSetupConnection(iofwdevent::CBException e)
        {
            e.check();
            setNextMethod(&RPCServerSM<InStream,OutStream>::postEncodeData);
        }

        void postEncodeData(iofwdevent::CBException e)
        {
            e.check();

            /* create the encoder */
            e_.coder_ = rpc::RPCEncoder(e_.data_ptr_, e_.data_size_);

            process(e_.coder_, e_.data_);

            e_.zero_copy_stream_->rewindOutput(e_.net_data_size_ - e_.coder_.getPos(), slots_[BASE_SLOT]);

            slots_.wait(BASE_SLOT,
                    &RPCServerSM<InStream,OutStream>::waitEncodeData);
        }

        void waitEncodeData(iofwdevent::CBException e)
        {
            e.check();
            setNextMethod(&RPCServerSM<InStream,OutStream>::postFlush);
        }

        void postFlush(iofwdevent::CBException e)
        {
            e.check();

            /* flush the connection */
            e_.zero_copy_stream_->flush(slots_[BASE_SLOT]);

            slots_.wait(BASE_SLOT,
                    &RPCServerSM<InStream,OutStream>::waitFlush);
        }

        void waitFlush(iofwdevent::CBException e)
        {
            e.check();
            setNextMethod(&RPCServerSM<InStream,OutStream>::postDecodeData);
        }

        void postDecodeData(iofwdevent::CBException e)
        {
            e.check();

            d_.zero_copy_stream_.reset((rpc_handle_->getIn()));

            /* get the max size */
            d_.net_data_size_ = rpc::getRPCEncodedSize(OutStream()).getMaxSize();

            d_.zero_copy_stream_->read(const_cast<const void **>(&d_.data_ptr_),
                    &d_.data_size_, slots_[BASE_SLOT], d_.net_data_size_);

            slots_.wait(BASE_SLOT,
                    &RPCServerSM<InStream,OutStream>::waitDecodeData);
        }

        void waitDecodeData(iofwdevent::CBException e)
        {
            e.check();

            d_.coder_ = rpc::RPCDecoder(d_.data_ptr_, d_.data_size_);

            process(d_.coder_, d_.data_);

            if(d_.coder_.getPos() != d_.data_size_)
            {
                fprintf(stderr, "%s:%i ERROR undecoded data...\n", __func__,
                        __LINE__);
            }
        }

    protected:
        /* SM */
        enum {BASE_SLOT = 0, NUM_BASE_SLOTS};
        sm::SimpleSlots<NUM_BASE_SLOTS,
            iofwdclient::clientsm::RPCServerSM <InStream,OutStream> > slots_;
        const iofwdevent::CBType & cb_;

        /* RPC */
        const std::string rpc_func_;
        boost::shared_ptr<iofwd::RPCClient> rpc_client_;
        rpc::RPCClientHandle rpc_handle_;
        net::AddressPtr addr_;

        /* encoder */
        RPCServerHelper<rpc::RPCEncoder, iofwdevent::ZeroCopyOutputStream,
            const InStream> e_;

        /* decoder */
        RPCServerHelper<rpc::RPCDecoder, iofwdevent::ZeroCopyInputStream,
            OutStream> d_;
};

    }
}

#endif
