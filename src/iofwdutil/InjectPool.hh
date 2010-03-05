#ifndef __IOFWDUTIL_INJECTPOOL_HH__
#define __IOFWDUTIL_INJECTPOOL_HH__

#include <boost/pool/pool_alloc.hpp>

#include "iofwdutil/tools.hh"

/* generic boost pool allocator template */
namespace iofwdutil
{
    template <typename T>
    class InjectPool
    {
        public:
            inline void * operator new(size_t UNUSED(s))
            {
                return boost::fast_pool_allocator<T>::allocate();
            }

            inline void operator delete(void * ptr)
            {
                boost::fast_pool_allocator<T>::deallocate(static_cast<T *>(ptr));
            }
    };
}
#endif
