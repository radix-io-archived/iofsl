#ifndef NET_TCP_TCPINPUT_HH
#define NET_TCP_TCPINPUT_HH

#include "iofwdevent/ZeroCopyInputStream.hh"

#include "net/tcp/TCPConnection.hh"

#include <boost/shared_ptr.hpp>

namespace net
{
    namespace tcp
    {
        class TCPConnector;

        struct TCPInput : public iofwdevent::ZeroCopyInputStream
        {
            protected:
                friend class TCPConnector;

                TCPInput(boost::shared_ptr<TCPConnection> connection);

            public:

                // Return a pointer to data
                iofwdevent::Handle read (const void ** ptr, size_t * size,
                    const iofwdevent::CBType & cb, size_t suggested);

                // Put back some data into the stream
                iofwdevent::Handle rewindInput (size_t size,
                    const iofwdevent::CBType & cb);

                virtual ~TCPInput ();

            protected:
                void readComplete(const iofwdevent::CBType & cb,
                        const iofwdevent::CBException & e);

            protected:
                const void ** get_ptr_;
                size_t * get_size_;
                const void * rewindptr_;
                size_t rewindsize_;
                size_t rewindused_;
                char * cur_read_raw_;
                size_t max_packet_size_;
                boost::shared_ptr<TCPConnection> connection_;
                bool data_ready_;
        };
    }
}
#endif
