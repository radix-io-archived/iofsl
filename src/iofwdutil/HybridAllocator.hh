#ifndef __IOFWDUTIL_STACK_ALLOCATOR_HH__
#define __IOFWDUTIL_STACK_ALLOCATOR_HH__

#include <memory>

//
// HybridAllocator
//
//   allocates data from a buffer off the stack instead of the heap
//   only meant for fundamental data types and works best for small
//    allocations (not exceeding S)
//
namespace iofwdutil
{
    template < int S >
    class HybridAllocator
    {
        public:
            HybridAllocator() : ptr_(0), size_(S)
            {
            }

            ~HybridAllocator()
            {
            }

            //
            // mem allocation
            //   if there is mem avail in the mem_ buffer, allocate from it
            //   else, dynamically allocate it using new
            //   for speed, don't search for unallocated space in mem_
            //
            inline void * malloc(long unsigned int size)
            {
                // if there is not enough mem in this obj, dyn allocate some
                if(ptr_ + size >= size_)
                {
                    return new char[size];
                }

                // get the next available mem
                void * m = &mem_[ptr_];

                // update the
                ptr_ += size;

                return m;
            }

            //
            // mem deallocation
            //   if ptr was not from mem_, deallocate using delete
            //   for speed, don't try to add back space to mem_
            //
            inline void free(void * ptr)
            {
                if(!(&mem_[0] <= static_cast<char *>(ptr) && &mem_[S - 1] >= static_cast<char *>(ptr)))
                {
                    delete [] static_cast<char **>(ptr);
                }
            }

            //
            // template malloc for the HA
            //
            // must call with hmalloc<typename>(size) so that compiler
            //  can deduce what template to use
            //
            template <typename T>
            T * hamalloc(unsigned long int size)
            {
                // if there is not enough mem in this obj, dyn allocate some
                if(ptr_ + (size * sizeof(T)) >= size_)
                {
                    return new T[size];
                }

                // get the next available mem
                T * m = reinterpret_cast<T *>(&mem_[ptr_]);

                // update the
                ptr_ += (size * sizeof(T));

                return m;
            }

            //
            // template free for the HA
            //
            template <typename T>
            void hafree(T * ptr)
            {
                if(!(reinterpret_cast<T *>(&mem_[0]) <= ptr && reinterpret_cast<T *>(&mem_[S - 1]) >= ptr))
                {
                    delete [] ptr;
                }
            }

        protected:
            char mem_[S];
            unsigned long int ptr_;
            unsigned long int size_;
    };
}

#endif
