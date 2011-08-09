#include "TCPConnector.hh"

#include "TCPAddress.hh"
#include "TCPInput.hh"
#include "TCPOutput.hh"

#include "iofwdutil/assert.hh"
#include "iofwdutil/tools.hh"

#include "net/tcp/TCPConnection.hh"

#include "iofwdevent/SingleCompletion.hh"

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread.hpp>

#include <iostream>

namespace net
{
    namespace tcp
    {
        boost::asio::io_service TCPConnector::io_service_;
        boost::asio::ip::tcp::acceptor * TCPConnector::g_acceptor_ = NULL;
        boost::mutex TCPConnector::io_service_mutex_;
        bool TCPConnector::io_service_active_ = false;
        bool TCPConnector::acceptor_set_ = false;

        void TCPConnector::run()
        {
            TCPConnector::io_service_.run();
        }

        TCPConnector::TCPConnector(std::string a) :
            acceptor_(NULL)
        {
            /* split the add into host and port */
            std::vector<std::string> addrstrs;
            boost::split(addrstrs, a, boost::is_any_of(":"));
            host_ = addrstrs[0];
            port_ = boost::lexical_cast<int>(addrstrs[1]);

            {
                boost::mutex::scoped_lock l(TCPConnector::io_service_mutex_);
                if(!TCPConnector::acceptor_set_)
                {
                    TCPConnector::g_acceptor_ = new
                        boost::asio::ip::tcp::acceptor(TCPConnector::io_service_, 
                                boost::asio::ip::tcp::endpoint(
                                    boost::asio::ip::address::from_string(host_),
                                    port_));
                    TCPConnector::acceptor_set_ = true;
                    acceptor_ = g_acceptor_;

                    boost::shared_ptr<TCPConnection> newConnection =
                                        TCPConnection::create(acceptor_->io_service());
                    acceptor_->async_accept(newConnection->socket(),
                            boost::bind(&TCPConnector::setupAsyncAcceptHandler,
                            this,
                            newConnection,
                            boost::asio::placeholders::error));
                }
                else
                {
                    acceptor_ = g_acceptor_;
                }
            
                if(!TCPConnector::io_service_active_)
                {
                    boost::thread runThread(&TCPConnector::run, this);

                    TCPConnector::io_service_active_ = true;
                }
            }
        }

        void TCPConnector::setupAsyncAcceptHandler(
                boost::shared_ptr<TCPConnection> connection,
                const boost::system::error_code &)
        {
            AcceptInfo info;

            std::string rhost =
                connection->socket().remote_endpoint().address().to_string();
            int rport = connection->socket().remote_endpoint().port();

            info.source = new TCPAddress(rhost, rport);
            info.in = new TCPInput(connection);
            info.out = new TCPOutput(connection, DEFAULT_BLOCKSIZE,
                    MAX_BLOCKSIZE);

            boost::shared_ptr<TCPConnection> newConnection =
                TCPConnection::create(acceptor_->io_service());

            acceptor_->async_accept(newConnection->socket(),
                    boost::bind(&TCPConnector::setupAsyncAcceptHandler,
                        this,
                        newConnection,
                        boost::asio::placeholders::error));
            accept_(info);
        }

        TCPConnector::~TCPConnector()
        {
        }

        void TCPConnector::setAcceptHandler(
                const AcceptHandler & h)
        {
            boost::mutex::scoped_lock l(lock_);
            accept_ = h;
        }

        void TCPConnector::lookup(const char * location,
                AddressPtr * ptr,
                const iofwdevent::CBType & cb)
        {
            ALWAYS_ASSERT(location);
            ALWAYS_ASSERT(ptr);

            std::vector<std::string> addrparts;

            boost::split(addrparts, location, boost::is_any_of(":"));

            /* set the addr */
            (*ptr) = new TCPAddress(addrparts[0],
                    boost::lexical_cast<unsigned int>(addrparts[1]));

            cb(iofwdevent::CBException ());
            
            return;
        }

        Connection TCPConnector::connect(const ConstAddressPtr & addr)
        {
            const TCPAddress & taddr = dynamic_cast<const TCPAddress &>(*addr);
            boost::shared_ptr<TCPConnection> c =
                TCPConnection::create(acceptor_->io_service());

            boost::asio::ip::tcp::resolver r(acceptor_->io_service());
            boost::asio::ip::tcp::resolver::query q(taddr.host_,
                    boost::lexical_cast<std::string>(taddr.port_));
            boost::asio::ip::tcp::resolver::iterator destination =
                    r.resolve(q);
            boost::system::error_code error = boost::asio::error::host_not_found;
            boost::asio::ip::tcp::endpoint endpoint; 

            {
                while(true)
                {
                    while(error && destination !=
                        boost::asio::ip::tcp::resolver::iterator())
                    {
                        endpoint = *destination++;
                    }

                    c->socket().shutdown(boost::asio::ip::tcp::socket::shutdown_both, error);
                    c->socket().close();
                    c->socket().connect(endpoint, error);

                    if(error)
                    {
                        destination = r.resolve(q);
                    }
                    else
                    {
                        break;
                    }
                }
            }

            return Connection(
                    new TCPInput(c),
                    new TCPOutput(c, DEFAULT_BLOCKSIZE, MAX_BLOCKSIZE));
        }

        void TCPConnector::newQuery (ZeroCopyInputStream * in,
                ZeroCopyOutputStream * out)
        {
            boost::mutex::scoped_lock l(lock_);

            if(!accept_)
            {
                l.unlock ();
                delete (in);
                delete (out);
                return;
            }

            AcceptInfo info;
            info.source = tcpaddr_;
            info.in = in;
            info.out = out;

            //accept_(info);
        }
   }
}
