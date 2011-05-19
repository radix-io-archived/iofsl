#ifndef EXTRASERVICE_AARPC_ATOMICAPPENDRPCTYPES_HH
#define EXTRASERVICE_AARPC_ATOMICAPPENDRPCTYPES_HH

#include "zoidfs/zoidfs.h"
#include "zoidfs/util/zoidfs-xdr.hh"
#include "iofwdutil/tools.hh"
#include "encoder/Util.hh"

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid_generators.hpp>

using namespace encoder;
using namespace encoder::xdr;

namespace iofwd
{
    namespace extraservice
    {
        /* AARPC data types */

        class AARPCCreateUUIDIn
        {
            public:
                AARPCCreateUUIDIn()
                {
                }

                zoidfs::zoidfs_handle_t handle;
        };

        class AARPCCreateUUIDOut
        {
            public:
                AARPCCreateUUIDOut() :
                    retcode(0),
                    huuid(boost::uuids::nil_uuid())
                {
                }

                uint64_t retcode;
                boost::uuids::uuid huuid;
        };

        class AARPCSeekOffsetIn
        {
            public:
                AARPCSeekOffsetIn() :
                    huuid(boost::uuids::nil_uuid())
                {
                }

                zoidfs::zoidfs_handle_t handle;
                boost::uuids::uuid huuid;
                zoidfs::zoidfs_file_size_t offset;
        };

        class AARPCSeekOffsetOut
        {
            public:
                AARPCSeekOffsetOut() :
                    retcode(0)
                {
                }

                uint64_t retcode;
        };

        class AARPCCreateOffsetIn
        {
            public:
                AARPCCreateOffsetIn() :
                    huuid(boost::uuids::nil_uuid())
                {
                }

                zoidfs::zoidfs_handle_t handle;
                boost::uuids::uuid huuid;
        };

        class AARPCCreateOffsetOut
        {
            public:
                AARPCCreateOffsetOut() :
                    retcode(0),
                    offset(0)
                {
                }

                uint64_t retcode;
                zoidfs::zoidfs_file_size_t offset;
        };

        class AARPCDeleteOffsetIn
        {
            public:
                AARPCDeleteOffsetIn() :
                    huuid(boost::uuids::nil_uuid())
                {
                }

                zoidfs::zoidfs_handle_t handle;
                boost::uuids::uuid huuid;

        };

        class AARPCDeleteOffsetOut
        {
            public:
                AARPCDeleteOffsetOut() :
                    retcode(0)
                {
                }

                uint64_t retcode; 

        };

        class AARPCGetNextOffsetIn
        {
            public:
                AARPCGetNextOffsetIn() :
                    huuid(boost::uuids::nil_uuid()),
                    inc(0)
                {
                }

                zoidfs::zoidfs_handle_t handle;
                boost::uuids::uuid huuid;
                zoidfs::zoidfs_file_size_t inc; 

        };

        class AARPCGetNextOffsetOut
        {
            public:
                AARPCGetNextOffsetOut() :
                    retcode(0),
                    offset(0)
                {
                }

                uint64_t retcode;
                zoidfs::zoidfs_file_ofs_t offset;
        };
       
        /* processors for the RPC in / out types */

        template <typename Enc, typename Wrapper>
        inline Enc & process (Enc & e, Wrapper & w, 
                typename process_filter<Wrapper, AARPCCreateUUIDIn>::type *
                UNUSED(d) = NULL)
        {
            process(e, w.handle);
            
            return e;
        }
        
        template <typename Enc, typename Wrapper>
        inline Enc & process (Enc & e, Wrapper & w,
                typename process_filter<Wrapper, AARPCCreateUUIDOut>::type *
                UNUSED(d) = NULL)
        {
            process(e, w.retcode);
            process(e, EncString(to_string(w.huuid).c_str(), 64));
            
            return e;
        }

        template <typename Enc, typename Wrapper>
        inline Enc & process (Enc & e, Wrapper & w, 
                typename process_filter<Wrapper, AARPCSeekOffsetIn>::type *
                UNUSED(d) = NULL)
        {
            process(e, w.handle);
            process(e, EncString(to_string(w.huuid).c_str(), 64));
            process(e, w.offset);
            
            return e;
        }
        
        template <typename Enc, typename Wrapper>
        inline Enc & process (Enc & e, Wrapper & w,
                typename process_filter<Wrapper, AARPCSeekOffsetOut>::type *
                UNUSED(d) = NULL)
        {
            process(e, w.retcode);
            
            return e;
        }

        template <typename Enc, typename Wrapper>
        inline Enc & process (Enc & e, Wrapper & w, 
                typename process_filter<Wrapper, AARPCGetNextOffsetIn>::type *
                UNUSED(d) = NULL)
        {
            process(e, w.handle);
            process(e, EncString(to_string(w.huuid).c_str(), 64));
            process(e, w.inc);
            
            return e;
        }
        
        template <typename Enc, typename Wrapper>
        inline Enc & process (Enc & e, Wrapper & w,
                typename process_filter<Wrapper, AARPCGetNextOffsetOut>::type *
                UNUSED(d) = NULL)
        {
            process(e, w.retcode);
            process(e, w.offset);
            
            return e;
        }

        template <typename Enc, typename Wrapper>
        inline Enc & process (Enc & e, Wrapper & w, 
                typename process_filter<Wrapper, AARPCCreateOffsetIn>::type *
                UNUSED(d) = NULL)
        {
            process(e, w.handle);
            process(e, EncString(to_string(w.huuid).c_str(), 64));
            
            return e;
        }
        
        template <typename Enc, typename Wrapper>
        inline Enc & process (Enc & e, Wrapper & w,
                typename process_filter<Wrapper, AARPCCreateOffsetOut>::type *
                UNUSED(d) = NULL)
        {
            process(e, w.retcode);
            process(e, w.offset);
            
            return e;
        }

        template <typename Enc, typename Wrapper>
        inline Enc & process (Enc & e, Wrapper & w, 
                typename process_filter<Wrapper, AARPCDeleteOffsetIn>::type *
                UNUSED(d) = NULL)
        {
            process(e, w.handle);
            process(e, EncString(to_string(w.huuid).c_str(), 64));
            
            return e;
        }
        
        template <typename Enc, typename Wrapper>
        inline Enc & process (Enc & e, Wrapper & w,
                typename process_filter<Wrapper, AARPCDeleteOffsetOut>::type *
                UNUSED(d) = NULL)
        {
            process(e, w.retcode);
            
            return e;
        }
    }
}

#endif
