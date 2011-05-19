#ifndef SRC_NET_TCP_TCPSERVICE_HH
#define SRC_NET_TCP_TCPSERVICE_HH

#include <string>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>

#include "net/tcp/TCPConnection.hh"

namespace net
{
    namespace tcp
    {

class TCPService
{
    public:
        TCPService(boost::asio::io_service & io_service) :
            acceptor_(io_service,
                    boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(),
                        720))
        {
            startAccept();
        }

        ~TCPService();

    private:
        void startAccept()
        {
                boost::shared_ptr<TCPConnection> newConnection =
                    TCPConnection::create(acceptor_.io_service());

                acceptor_.async_accept(newConnection->socket(),
                        boost::bind(&TCPService::handleAccept, this,
                            newConnection, boost::asio::placeholders::error));
        }

        void handleAccept(boost::shared_ptr<TCPConnection> newConnection,
                const boost::system::error_code & error)
        {
            if(!error)
            {
                //newConnection->post_recv();
                startAccept();
            }
        }

        boost::asio::ip::tcp::acceptor acceptor_;
};
    }
}

#endif
