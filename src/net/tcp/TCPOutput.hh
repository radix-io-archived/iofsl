#ifndef NET_TCP_TCPOUTPUT_HH
#define NET_TCP_TCPOUTPUT_HH

#include "iofwdevent/ZeroCopyOutputStream.hh"

#include "net/tcp/TCPConnection.hh"

#include <boost/shared_ptr.hpp>

namespace net
{
    namespace tcp
    {

        class TCPConnector;

        class TCPOutput : public iofwdevent::ZeroCopyOutputStream
        {
            protected:
                friend class TCPConnector;

                TCPOutput(boost::shared_ptr<TCPConnection> connection,
                        size_t def_blocksize,
                        size_t max_blocksize);

            public:
                iofwdevent::Handle write(void ** ptr, size_t * size,
                    const iofwdevent::CBType & cb, size_t suggested);

                iofwdevent::Handle rewindOutput(size_t size,
                    const iofwdevent::CBType & cb);

                void internal_flush(const iofwdevent::CBType & cb);
                iofwdevent::Handle flush(const iofwdevent::CBType & cb);

                virtual ~TCPOutput();

            protected:
                void internal_flush();
                void internal_flush_done(void * buffer,
                        const iofwdevent::CBType & cb,
                        iofwdevent::CBException e);

            protected:
                size_t def_blocksize_;
                size_t max_blocksize_;
                void * thisblock_ptr_;
                size_t thisblock_size_;
                size_t thisblock_used_;

                boost::shared_ptr<TCPConnection> connection_;
        };
    }
}
#endif
