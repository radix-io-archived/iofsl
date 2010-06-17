#ifndef IOFWDUTIL_INJECTPOOL_HH
#define IOFWDUTIL_INJECTPOOL_HH

#include <boost/pool/pool_alloc.hpp>

#include "iofwdutil/assert.hh"
#include "iofwdutil/tools.hh"

/* generic boost pool allocator template */
namespace iofwdutil
{
    template <typename T>
    class InjectPool
    {
        public:
            inline void * operator new(size_t s)
            {
               // This check is here to make sure nobody inherits from T
               ASSERT(s == sizeof(T));
               return boost::fast_pool_allocator<T>::allocate();
            }

            inline void operator delete(void * ptr)
            {
                boost::fast_pool_allocator<T>::deallocate(static_cast<T *>(ptr));
            }
    };
}
#endif
