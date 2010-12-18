#ifndef ENCODER_NONE_READER_HH
#define ENCODER_NONE_READER_HH

#include "iofwdutil/assert.hh"

#include "encoder/EncoderException.hh"

#include "encoder/Decoder.hh"
#include "encoder/EncoderWrappers.hh"

namespace encoder
{
   namespace none
   {
      //=====================================================================

      struct Reader : public Decoder
      {
         public:

            Reader ()
               : mem_(0), maxbytes_(0), pos_(0)
            {
            }

            Reader (const void * mem, size_t maxmem)
            {
               reset (mem, maxmem);
            }

            size_t getMaxSize () const
            { return maxbytes_; }

            size_t getPos () const
            { return pos_; }

            void reset (const void * mem, size_t maxmem)
            {
               mem_ = static_cast<const char*>(mem);
               maxbytes_ = maxmem;
               pos_ = 0;
            }

            void rewind (size_t newpos = 0)
            {
               ASSERT(newpos < maxbytes_);
               pos_= newpos;
            }

         public:
            // public to avoid having to list every friend function

            void get (void * dest, size_t bytes)
            {
               if (pos_ + bytes > maxbytes_)
               {
                  ZTHROW (BufferException ()
                     << iofwdutil::zexception_msg("Out "
                        "of bufferspace while decoding"));
               }
               memcpy (dest, &mem_[pos_], bytes);
               pos_ += bytes;
            }

         protected:
            const char * mem_;
            size_t maxbytes_;
            size_t pos_;
      };

      /**
       * We define how to read certain basic types, and rely on the overload
       * of the process function to support the rest.
       */

      //=====================================================================

#define READER_SIMPLE(type) \
      inline void process (Reader & r, type & out) \
      { r.get (&out, sizeof(type)); }

READER_SIMPLE(uint8_t);
READER_SIMPLE(uint16_t);
READER_SIMPLE(uint32_t);
READER_SIMPLE(uint64_t);

READER_SIMPLE(int8_t);
READER_SIMPLE(int16_t);
READER_SIMPLE(int32_t);
READER_SIMPLE(int64_t);

READER_SIMPLE(char);

// uint8_t is same as unsigned char
//READER_SIMPLE(unsigned char);

// int8_t is same as signed char
// READER_SIMPLE(signed char);

#undef READER_SIMPLE

      inline void process (Reader & r, const EncString & s)
      {
         uint32_t len;
         process (r, len);
         if (len > s.maxsize_)
         {
            ZTHROW (TypeException () <<
                  iofwdutil::zexception_msg("Mismatch in string length!"));
         }
         r.get (s.ptr_, len);
         s.ptr_[len] = 0;
      }

      // Consider adding debug mode where we also encode length to make sure
      // encoding and decoding agree on the length of the opaque data
      inline void process (Reader & r, const EncOpaque & o)
      {
         r.get (o.ptr_, o.size_);
      }

      template <typename T, typename C>
      inline void process (Reader & r, const EncVarArrayHelper<T,C> & a)
      {
         uint32_t count;
         process (r, count);
         if (count > a.maxcount_)
         {
            ZTHROW (TypeException () << iofwdutil::zexception_msg(
                     "Array truncated while decoding VarArray"));
         }
         a.count_ = count;
         for (size_t i=0; i<count; ++i)
         {
            process (r, a.ptr_[i]);
         }
      }

      template <typename T>
      inline void process (Reader & r, const EncEnumHelper<T> & e)
      {
         uint32_t i;
         STATIC_ASSERT(sizeof(T) <= sizeof(uint32_t));
         process (r, i);
         e.enum_ = static_cast<T>(i);
      }

      //=====================================================================
   }
}

#endif
