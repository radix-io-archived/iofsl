#ifndef NET_TCP_TCPADDRESS_HH
#define NET_TCP_TCPADDRESS_HH

#include "net/Address.hh"

#include <string>

namespace net
{
    namespace tcp
    {
        class TCPAddress : public Address
        {
            public:
                TCPAddress(std::string host=std::string(""),
                        unsigned int port=0) :
                    port_(port),
                    host_(host)
                {
                }

                virtual ~TCPAddress();

                unsigned int port_;
                std::string host_;
        };
    }
}

#endif

