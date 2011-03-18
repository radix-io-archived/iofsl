#ifndef ENCODER_NONE_SIZE_HH
#define ENCODER_NONE_SIZE_HH

#include "iofwdutil/assert.hh"

namespace encoder
{
   namespace none
   {
      //=====================================================================

      struct Size : public ::encoder::Size
      {

      };

      //=====================================================================

#define NONESIZE_SIMPLE(type) \
      inline void process (::encoder::none::Size & s, const type & ) \
      { s.incBoth (sizeof (type)); }

NONESIZE_SIMPLE(uint8_t);
NONESIZE_SIMPLE(uint16_t);
NONESIZE_SIMPLE(uint32_t);
NONESIZE_SIMPLE(uint64_t);

NONESIZE_SIMPLE(int8_t);
NONESIZE_SIMPLE(int16_t);
NONESIZE_SIMPLE(int32_t);
NONESIZE_SIMPLE(int64_t);

NONESIZE_SIMPLE(char);

// [u]int8_t is same as [u]char
//NONESIZE_SIMPLE(unsigned char);
//NONESIZE_SIMPLE(signed char);

NONESIZE_SIMPLE(double);
NONESIZE_SIMPLE(float);

#undef NONESIZE_SIMPLE

       inline void process (::encoder::none::Size & s, const EncString & str)
       {
          s.incBoth (sizeof(uint32_t));
          if (s.isActualValid ())
          {
             s.incActualSize (strlen (str.ptr_));
          }
          s.incMaxSize (str.maxsize_);
       }

       inline void process (::encoder::none::Size & s, const EncOpaque & e)
       {
          s.incMaxSize ( e.maxsize_);
          s.incActualSize ( e.size_);
       }

       template <typename T>
       inline void process (::encoder::none::Size & s,
             const EncEnumHelper<T> & )
       {
          STATIC_ASSERT(sizeof(T) <= sizeof(uint32_t));
          s.incBoth (sizeof(uint32_t));
       }

       template <typename T, typename C>
       inline void process (::encoder::none::Size & s,
             const EncVarArrayHelper<T,C> & a)
       {
          s.incBoth (sizeof(uint32_t)); // store count
          STATIC_ASSERT(false && sizeof(C));
       }

      //=====================================================================
   }
}

#endif
