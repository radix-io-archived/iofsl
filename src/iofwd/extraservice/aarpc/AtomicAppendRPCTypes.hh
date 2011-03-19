#ifndef EXTRASERVICE_AARPC_ATOMICAPPENDRPCTYPES_HH
#define EXTRASERVICE_AARPC_ATOMICAPPENDRPCTYPES_HH

#include "zoidfs/zoidfs.h"
#include "zoidfs/util/zoidfs-xdr.hh"
#include "iofwdutil/tools.hh"
#include "encoder/Util.hh"

using namespace encoder;
using namespace encoder::xdr;

namespace iofwd
{
    namespace extraservice
    {
        /* AARPC data types */

        class AARPCCreateOffsetIn
        {
            public:
                AARPCCreateOffsetIn()
                {
                }

                zoidfs::zoidfs_handle_t handle;
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
                AARPCDeleteOffsetIn()
                {
                }

                zoidfs::zoidfs_handle_t handle;

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
                    inc(0)
                {
                }

                zoidfs::zoidfs_handle_t handle;
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
                typename process_filter<Wrapper, AARPCGetNextOffsetIn>::type *
                UNUSED(d) = NULL)
        {
            process(e, w.handle);
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
