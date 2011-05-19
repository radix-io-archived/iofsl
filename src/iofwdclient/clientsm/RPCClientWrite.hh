#ifndef IOFWDCLIENT_CLIENTSM_RPCCLIENTWRITE_HH
#define IOFWDCLIENT_CLIENTSM_RPCCLIENTWRITE_HH

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
#include "iofwdevent/SingleCompletion.hh"
#include "iofwdevent/ZeroCopyInputStream.hh"
#include "iofwdevent/ZeroCopyOutputStream.hh"

#include "zoidfs/zoidfs.h"
#include "zoidfs/zoidfs-rpc.h"

#include "rpc/RPCEncoder.hh"
#include "iofwd/Net.hh"

#include "net/Address.hh"
#include "net/Net.hh"

#include <cstdio>

using namespace iofwdclient;
using namespace iofwdclient::streamwrappers;
using namespace encoder;
using namespace encoder::xdr;

namespace iofwdclient
{
    namespace clientsm
    {

template< typename INTYPE, typename OUTTYPE >
class RPCClientWrite :
    public sm::SimpleSM< iofwdclient::clientsm::RPCClientWrite <INTYPE, OUTTYPE> >
{
    public:
        RPCClientWrite(sm::SMManager & smm,
                       bool poll,
                       const iofwdevent::CBType & cb,
                       rpc::RPCClientHandle rpc_handle,
                       const INTYPE & in,
                       OUTTYPE & out) :
            sm::SimpleSM< iofwdclient::clientsm::RPCClientWrite <INTYPE,OUTTYPE> >(smm, poll),
            slots_(*this),
            e_(in),
            d_(out),
            rpc_handle_(rpc_handle)
        {
            inStream_.reset( new INTYPE(static_cast<const zoidfs::zoidfs_handle_t*>(in.handle_),
                                        static_cast<size_t>(in.mem_count_), 
                                        static_cast<const void **>(in.mem_starts_),
                                        static_cast<const size_t *>(in.mem_sizes_),
                                        static_cast<size_t>(in.file_count_), 
                                        static_cast<const zoidfs::zoidfs_file_ofs_t*>(in.file_starts_),
                                        static_cast<zoidfs::zoidfs_file_ofs_t*>(in.file_sizes_), 
                                        (zoidfs::zoidfs_op_hint_t*) NULL));
            cb_ = (iofwdevent::CBType)cb;
        }

        ~RPCClientWrite()
        {
//	   fprintf(stderr,"Im Being Destroyed\n");
        }

        void init(iofwdevent::CBException e)
        {
            e.check();
            /* this is an error, causes e_.data_ to be freed early */
            /* Get the maximum possible send size */
            e_.net_data_size_ = rpc::getRPCEncodedSize(e_.data_).getMaxSize();
            /* Set the output stream */
            rpc_handle_->waitOutReady (slots_[BASE_SLOT]);
            slots_.wait (BASE_SLOT, 
                       &RPCClientWrite<INTYPE,OUTTYPE>::outputReady);
        }

        void outputReady (iofwdevent::CBException e)
        {
            e.check();
         
            e_.zero_copy_stream_.reset((rpc_handle_->getOut()));

            setNextMethod(&RPCClientWrite<INTYPE,OUTTYPE>::postSetupConnection);
        }

        void postSetupConnection(iofwdevent::CBException e)
        {
         
            e.check();

            /* setup the write stream */
            e_.zero_copy_stream_->write(&e_.data_ptr_, &e_.data_size_, slots_[BASE_SLOT],
                    e_.net_data_size_);

            slots_.wait (BASE_SLOT, 
                       &RPCClientWrite<INTYPE,OUTTYPE>::waitSetupConnection);
        }

        void waitSetupConnection(iofwdevent::CBException e)
        {
         
            e.check();
            setNextMethod(&RPCClientWrite<INTYPE,OUTTYPE>::postEncodeData);
        }

        void postEncodeData(iofwdevent::CBException e)
        {
//	    fprintf(stderr, "Posting Encode\n");         
            e.check();

            /* create the encoder */
            e_.coder_ = rpc::RPCEncoder(e_.data_ptr_, e_.data_size_);

            process(e_.coder_, e_.data_);

            e_.zero_copy_stream_->rewindOutput(e_.data_size_ - e_.coder_.getPos(), slots_[BASE_SLOT]);

            slots_.wait(BASE_SLOT,
                    &RPCClientWrite<INTYPE,OUTTYPE>::flushWriteBuffer);

        }

        /* Get the write buffer for writing data */
        void getWriteBuffer (iofwdevent::CBException e)
        {
//	    fprintf(stderr,"Got Write Buffer\n");
            e.check();         

            /* setup the write stream */
            e_.zero_copy_stream_->write(&e_.data_ptr_, &e_.data_size_, slots_[BASE_SLOT],
                    RemainingWrite(inStream_.get()));

            slots_.wait(BASE_SLOT,&RPCClientWrite<INTYPE,OUTTYPE>::writeData);
        }

        void flushWriteBuffer (iofwdevent::CBException e)
        {
            e.check();
//	    fprintf(stderr, "Flushing Write Buffer\n");
            e_.zero_copy_stream_->flush(slots_[BASE_SLOT]);
            slots_.wait(BASE_SLOT,
                    &RPCClientWrite<INTYPE,OUTTYPE>::getWriteBuffer);  
        }          
      
        /* Write data to output stream */
        void writeData (iofwdevent::CBException e)
        {
            e.check();
            
            size_t outSize = e_.data_size_;
//	    fprintf(stderr, "Output Size: %i\n", outSize);
            int ret = getWriteData (&e_.data_ptr_, &outSize, inStream_.get());
            /* write finished */
            if (ret == 0)
            {
               /* rewind then continue to flush */
               e_.zero_copy_stream_->rewindOutput(e_.data_size_ - outSize, slots_[BASE_SLOT]);
               slots_.wait(BASE_SLOT,
                           &RPCClientWrite<INTYPE,OUTTYPE>::waitEncodeData); 
            }
            /* More data to be written (out of write buffer) */
            else 
            {
    //           fprintf(stderr,"Going to next write\n");
               setNextMethod(&RPCClientWrite<INTYPE,OUTTYPE>::flushWriteBuffer);               
            }
        }

        /* Flush state */
        void waitEncodeData(iofwdevent::CBException e)
        {
            e.check();
            setNextMethod(&RPCClientWrite<INTYPE,OUTTYPE>::postFlush);
        }

        void postFlush(iofwdevent::CBException e)
        {
  //          fprintf(stderr, "Posting Flush\n");         
            e.check();

            // Before we can access the output channel we need to wait until the RPC
            // code did its thing

            if (e_.zero_copy_stream_->type == 'T')            
               e_.zero_copy_stream_->close(slots_[BASE_SLOT]);   
            else
               e_.zero_copy_stream_->flush(slots_[BASE_SLOT]);
            slots_.wait(BASE_SLOT,
                    &RPCClientWrite<INTYPE,OUTTYPE>::waitFlush);
        }

        void waitFlush(iofwdevent::CBException e)
        {
         
            e.check();
            rpc_handle_->waitInReady (slots_[BASE_SLOT]);
            slots_.wait(BASE_SLOT,&RPCClientWrite<INTYPE,OUTTYPE>::postDecodeData);
        }

        
        void postDecodeData(iofwdevent::CBException e)
        {
         
            e.check();      
            /* get the max size */
            d_.net_data_size_ = rpc::getRPCEncodedSize(OUTTYPE()).getMaxSize();
            d_.zero_copy_stream_.reset((rpc_handle_->getIn()));
            d_.zero_copy_stream_->read(const_cast<const void **>(&d_.data_ptr_),
                    &d_.data_size_, slots_[BASE_SLOT], d_.net_data_size_);

            slots_.wait(BASE_SLOT,
                    &RPCClientWrite<INTYPE,OUTTYPE>::waitDecodeData);
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
            
            cb_ (e);
        }

    protected:
        /* SM */
        enum {BASE_SLOT = 0, NUM_BASE_SLOTS};
        sm::SimpleSlots<NUM_BASE_SLOTS,
            iofwdclient::clientsm::RPCClientWrite <INTYPE,OUTTYPE> > slots_;
        iofwdevent::CBType cb_;

        /* RPC */
        const std::string rpc_func_;
        boost::shared_ptr<iofwd::RPCClient> rpc_client_;

        /* encoder */
        RPCServerHelper<rpc::RPCEncoder, iofwdevent::ZeroCopyOutputStream,
            const INTYPE> e_;

        /* decoder */
        RPCServerHelper<rpc::RPCDecoder, iofwdevent::ZeroCopyInputStream,
            OUTTYPE> d_;

        rpc::RPCClientHandle rpc_handle_;
        
        boost::shared_ptr<INTYPE> inStream_;
};

    }
}

#endif
