#ifndef NET_TCP_TCPCONNECTOR_HH
#define NET_TCP_TCPCONNECTOR_HH

#include <boost/asio.hpp>
#include <boost/noncopyable.hpp>
#include <boost/thread.hpp>

#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>

#include "net/Net.hh"
#include "iofwdevent/CBType.hh"
#include "net/tcp/TCPConnection.hh"

#include "iofwdevent/SingleCompletion.hh"

namespace net
{
    namespace tcp
    {

class TCPConnector : private boost::noncopyable, public Net 
{
    protected:
        enum
        {
            DEFAULT_BLOCKSIZE = 1048576,
            MAX_BLOCKSIZE = 67108864
        };

    public:
        TCPConnector(std::string a);
        ~TCPConnector();


        void lookup(const char * addr, AddressPtr * ptr, 
                const iofwdevent::CBType & cb);

         Connection connect(const ConstAddressPtr & addr);
         
         void createGroup(GroupHandle *,
               const std::vector<std::string> &,
               const iofwdevent::CBType &)
         {
         }

        void newQuery(ZeroCopyInputStream * in,
                        ZeroCopyOutputStream * out);

         void setAcceptHandler(const AcceptHandler & h);
         void setupAsyncAcceptHandler(
                 boost::shared_ptr<TCPConnection> connection,
                 const boost::system::error_code & error);

         void run();

         std::string host_;
         int port_;

         AcceptHandler accept_;
         AddressPtr tcpaddr_;
         
         boost::mutex lock_;

         static boost::asio::io_service io_service_;
         static boost::mutex io_service_mutex_;
         static bool io_service_active_;
         static bool acceptor_set_;

         static boost::asio::ip::tcp::acceptor * g_acceptor_;
         boost::asio::ip::tcp::acceptor * acceptor_;
};
    }
}

#endif
