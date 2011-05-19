#ifndef SRC_NET_TCP_TCPCONNECTION_HH
#define SRC_NET_TCP_TCPCONNECTION_HH

#include <string>
#include <iostream>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>

#include "iofwdevent/CBType.hh"
#include "iofwdevent/CBException.hh"

namespace net
{
    namespace tcp
    {

class TCPConnection : public boost::enable_shared_from_this<TCPConnection>
{
    public:
        static boost::shared_ptr<TCPConnection> create(boost::asio::io_service & io_service)
        {
            return boost::shared_ptr<TCPConnection>(new TCPConnection(io_service));
        }

        boost::asio::ip::tcp::socket & socket()
        {
            return socket_;
        }

        void post_send(const iofwdevent::CBType & cb, void * b, size_t bsize)
        {
            boost::asio::async_write(socket_, boost::asio::buffer(b, bsize),
                    boost::bind(&TCPConnection::handleWrite,
                        shared_from_this(),
                        cb,
                        boost::asio::placeholders::error,
                        boost::asio::placeholders::bytes_transferred));
        }

        void post_recv(const iofwdevent::CBType & cb, void * b, size_t * bsize)
        {
            socket_.async_read_some(boost::asio::buffer(b, *bsize),
                    boost::bind(&TCPConnection::handleRead,
                        shared_from_this(),
                        cb,
                        bsize,
                        boost::asio::placeholders::error,
                        boost::asio::placeholders::bytes_transferred));
        }

        ~TCPConnection();

    protected:
        TCPConnection(boost::asio::io_service & io_service) :
            socket_(io_service)
        {
        }

        void handleWrite(const iofwdevent::CBType & cb, const
                boost::system::error_code &, size_t)
        {
            cb(iofwdevent::CBException());
        }

        void handleRead(const iofwdevent::CBType & cb, size_t * asize, 
                const boost::system::error_code &, size_t bytes)
        {
            *asize = bytes;
            cb(iofwdevent::CBException());
        }

        boost::asio::ip::tcp::socket socket_;
        std::string message_;
};
    }
}

#endif
