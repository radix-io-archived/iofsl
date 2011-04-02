#ifndef ENCODER_XDRWRITER_HH
#define ENCODER_XDRWRITER_HH

#include "XDRBase.hh"
#include "encoder/Encoder.hh"

namespace encoder
{
   namespace xdr
   {
//===========================================================================

class XDRWriter : public XDRBase, public Encoder
{
public:

   XDRWriter ()
      : XDRBase (0, 0, false)
   {
   }

   XDRWriter (void * mem, size_t memsize)
      : XDRBase (mem, memsize, false)
   {
   }

   const void * getBuf () const
   {
      return mem_;
   }

   /* Return actual encoded data size */
   size_t size () const
   {
      return getPos ();
   }

   template <typename T>
   XDRWriter & operator << (const T & val)
   {
      process (*this, val);
      return *this;
   }

   template <typename T>
   XDRWriter & operator () (const T & val)
   {
      process (*this, val);
      return *this;
   }

      /**
       * Return the maximum size of the buffer we're encoding to.
       */
      size_t getMaxSize () const
      { return XDRBase::getMaxSize(); }

      /**
       * Return how many bytes have been written to the buffer.
       */
      size_t getPos () const
      { return XDRBase::getPos(); }

      /**
       * Switch to a new buffer.
       */
      void reset (void * mem, size_t maxsize)
      { XDRBase::reset (mem, maxsize); }

      /**
       * Set next output location in buffer.
       */
      void rewind (size_t newpos = 0)
      { XDRBase::rewind (newpos); }


};

//===========================================================================

// We need the const cast since xdr_xxxxx is not const-safe.
#define XDRHELPER(type,func) \
   inline void process (XDRWriter & f, const type & s) \
   { f.check(xdr_##func (&f.xdr_, const_cast<type *>(&s)));  }

#define XDRHELPER2(type) XDRHELPER(type,type)

XDRHELPER2(char)

XDRHELPER2(uint8_t)
XDRHELPER2(uint16_t)
XDRHELPER2(uint32_t)
XDRHELPER2(uint64_t)

XDRHELPER2(int8_t)
XDRHELPER2(int16_t)
XDRHELPER2(int32_t)
XDRHELPER2(int64_t)


// bool_t is defined as int or some other integral type
// XDRHELPER(bool_t,bool);

#undef XDRHELPER


// Processor functions
inline void process (XDRWriter & f, const EncOpaque & o)
{
   f.check(xdr_opaque(&f.xdr_, (char*) o.ptr_, (unsigned int) o.size_));
}

inline void process (XDRWriter & f, const EncString & s)
{
   f.check(xdr_string(&f.xdr_, &s.ptr_, (unsigned int) s.maxsize_));
}


template <typename T, typename C>
inline void process (XDRWriter & f, const EncVarArrayHelper<T,C> & a)
{
   // send/receive array count
   uint32_t count = static_cast<uint32_t>(a.count_);
   process (f, count);
   /* This line doesnt make sense with the const EncVarArrayHelper */
   //a.count_ = count;

   ALWAYS_ASSERT (count <= a.maxcount_);

   // send/receive array elements
   for (unsigned int i=0; i<count; ++i)
      process (f, a.ptr_[i]);
}

template <typename T>
inline void process (XDRWriter & f, const EncEnumHelper<T> & e)
{
   BOOST_STATIC_ASSERT (sizeof (T) <= sizeof (enum_t));
   // might be faster here to skip the read/write test all together
   enum_t fixed = e.enum_;
   f.check(xdr_enum(&f.xdr_, &fixed));
}


//===========================================================================
    }
}

#endif
