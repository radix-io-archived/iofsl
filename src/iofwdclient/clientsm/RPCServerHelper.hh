#ifndef IOFWDCLIENT_CLIENTSM_RPCSERVERHELPER_HH
#define IOFWDCLIENT_CLIENTSM_RPCSERVERHELPER_HH

#include <boost/scoped_ptr.hpp>

namespace iofwdclient
{
    template<typename F, typename S, typename D>
    class RPCServerHelper
    {
        public:
            RPCServerHelper(D & data) :
                data_ptr_(NULL),
                data_size_(0),
                net_data_size_(0),
                zero_copy_stream_(NULL),
                data_(data)
            {
            }

            void * data_ptr_;
            size_t data_size_;
            size_t net_data_size_;
            boost::scoped_ptr<S> zero_copy_stream_;
            F coder_;
            D & data_;
    };
}

#endif
