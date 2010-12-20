#ifndef ENCODER_NONE_WRITER_HH
#define ENCODER_NONE_WRITER_HH

#include "iofwdutil/assert.hh"

#include "encoder/Encoder.hh"
#include "encoder/EncoderWrappers.hh"

namespace encoder
{
   namespace none
   {
      //=====================================================================

      struct Writer : public Encoder
      {
         public:

            Writer (void * mem, size_t maxmem)
            {
               reset (mem, maxmem);
            }

            Writer ()
               : mem_(0), maxbytes_(0), pos_(0)
            {
            }

            size_t getMaxSize () const
            { return maxbytes_; }

            size_t getPos () const
            { return pos_; }

            void reset (void * mem, size_t maxmem)
            {
               mem_ = static_cast<char*>(mem);
               maxbytes_ = maxmem;
               pos_ = 0;
            }

            void rewind (size_t newpos = 0)
            {
               ASSERT(newpos < maxbytes_);
               pos_= newpos;
            }

         public: // public to avoid having to enumerate all friend functions

            void put (const void * source, size_t bytes)
            {
               if (pos_ + bytes > maxbytes_)
               {
                  ZTHROW (BufferException ()
                     << iofwdutil::zexception_msg("Out "
                        "of bufferspace while encoding"));
               }
               memcpy (&mem_[pos_], source, bytes);
               pos_ += bytes;
            }

         protected:
            char * mem_;
            size_t maxbytes_;
            size_t pos_;
      };

      /**
       * We define how to write certain basic types, and rely on the overload
       * of the process function to support the rest.
       */

      //=====================================================================

#define WRITER_SIMPLE(type) \
      inline void process (Writer & r, const type & out) \
      { r.put (&out, sizeof(type)); }

WRITER_SIMPLE(uint8_t);
WRITER_SIMPLE(uint16_t);
WRITER_SIMPLE(uint32_t);
WRITER_SIMPLE(uint64_t);

WRITER_SIMPLE(int8_t);
WRITER_SIMPLE(int16_t);
WRITER_SIMPLE(int32_t);
WRITER_SIMPLE(int64_t);

WRITER_SIMPLE(char);

// [u]int8_t is same type as (u)char
//WRITER_SIMPLE(unsigned char);
//WRITER_SIMPLE(signed char);

#undef WRITER_SIMPLE

      inline void process (Writer & r, const EncString & s)
      {
         uint32_t len = strlen (s.ptr_);
         ASSERT (len <= s.maxsize_);
         process (r, len);
         r.put (s.ptr_, len);
      }

      inline void process (Writer & w, const EncOpaque & o)
      {
         w.put (o.ptr_, o.size_);
      }

      template <typename T, typename C>
      inline void process (Writer & w, const EncVarArrayHelper<T,C> & o)
      {
         uint32_t count = o.count_;
         ASSERT(count == o.count_); // check for overflow
         process (w, count);
         for (size_t i=0; i<count; ++i)
            process (w, o.ptr_[i]);
      }

      template <typename T>
      inline void process (Writer & r, const EncEnumHelper<T> & e)
      {
         uint32_t i = e.enum_;
         STATIC_ASSERT(sizeof(T) <= sizeof(uint32_t));
         process (r, i);
      }

      //=====================================================================
   }
}

#endif
