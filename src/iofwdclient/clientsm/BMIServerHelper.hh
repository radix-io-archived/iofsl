#ifndef IOFWDCLIENT_CLIENTSM_BMISERVERHELPER_HH
#define IOFWDCLIENT_CLIENTSM_BMISERVERHELPER_HH

namespace iofwdclient
{
    template<typename F, typename S, typename D>
    class BMIServerHelper
    {
        public:
            RPCServerHelper(D & data) :
                data_(data)
            {
            }

            F coder_;
            D & data_;
    };
}

#endif
