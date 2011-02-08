
#define BOOST_TEST_MODULE "Encoder Library"
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#include <boost/format.hpp>
#include <numeric>
#include <vector>
#include <algorithm>
#include <iostream>
#include <typeinfo>
#include <string.h>

#include <boost/mpl/list.hpp>
#include <boost/bind.hpp>
#include <boost/mpl/for_each.hpp>
#include <boost/smart_ptr.hpp> 

#include "iofwd_config.h"
#include "ThreadSafety.hh"

#include "zoidfs/util/zoidfs-xdr.hh"
#include "zoidfs/util/zoidfs-wrapped.hh"
#include "zoidfs/util/zoidfs-ops.hh"

#include "iofwdutil/Timer.hh"

#include "encoder/xdr/XDRSizeProcessor.hh"
#include "encoder/xdr/XDRReader.hh"
#include "encoder/xdr/XDRWriter.hh"

#include "encoder/SizeSaver.hh"
#include "encoder/EncoderException.hh"

#include "encoder/none/Reader.hh"
#include "encoder/none/Writer.hh"
#include "encoder/none/Size.hh"

#define MEMSIZE 32*1024*1024


// How long we measure speeds
#define MEASURETIME 2.0

#define MB (1024*1024)

using namespace encoder::xdr;
using namespace encoder;
using namespace std;
using namespace zoidfs;
using namespace boost;


// ==========================================================================
// ==== Helper to have a human readable name for types ======================
// ==========================================================================

// Helper for type->name mapping
template <typename T>
inline const char * getName ()
{ return typeid(T).name(); }

#define GETNAMEHELPER(a) \
   template <> inline const char * getName<a>() { return #a; }

GETNAMEHELPER(uint8_t)
GETNAMEHELPER(uint16_t)
GETNAMEHELPER(uint32_t)
GETNAMEHELPER(uint64_t)

GETNAMEHELPER(int8_t)
GETNAMEHELPER(int16_t)
GETNAMEHELPER(int32_t)
GETNAMEHELPER(int64_t)

GETNAMEHELPER(zoidfs_handle_t)
GETNAMEHELPER(zoidfs_attr_type_t)
GETNAMEHELPER(zoidfs_time_t)
GETNAMEHELPER(zoidfs_attr_t)
GETNAMEHELPER(zoidfs_cache_hint_t)
GETNAMEHELPER(zoidfs_sattr_t)
//GETNAMEHELPER(zoidfs_dirent_cookie_t)
GETNAMEHELPER(zoidfs_dirent_t)

GETNAMEHELPER(float)
GETNAMEHELPER(double)

#undef GETNAMEHELPER

   
// ==========================================================================
// ==========================================================================
// ==========================================================================

// The following defines lists of types we test for the generic tests.

typedef boost::mpl::list<uint8_t, uint16_t, uint32_t, uint64_t> UITypes;
typedef boost::mpl::list<int8_t, int16_t, int32_t, uint64_t>    ITypes;
typedef boost::mpl::list<zoidfs_handle_t, zoidfs_attr_type_t,
        zoidfs_time_t, zoidfs_attr_t, zoidfs_sattr_t,
        zoidfs_dirent_cookie_t, zoidfs_dirent_t>                ZTypes;


template <typename PROC>
void doGenericTests (PROC & p)
{
   mpl::for_each<ITypes> (p);
   mpl::for_each<UITypes> (p);
   mpl::for_each<ZTypes> (p);

   checkBufferProtection (p.size, p.writer, p.reader);
   

   // @TODO: add wrapper types (string, enum, array)
   testString (p.size, p.writer, p.reader);
}


// ==========================================================================
// ==================== HELPER FUNCTIONS FOR DEALING WITH TYPES =============
// ==========================================================================


void randomString (char * dst, size_t bufsize)
{
   const size_t len = random () % bufsize;
   for (size_t i=0; i<len; ++i)
      dst[i] = 'A' + (random () % 26);
   dst[len]=0;
}

// Initialize a type; Must be specialized for special types (i.e. string,
// ...)
template <typename T>
void per_type_init (T & t)
{
   char * ptr = (char*) &t;
   for (unsigned int i=0; i<sizeof(T); ++i)
      *ptr++ = static_cast<char>(random());
}

template <typename T>
bool per_type_compare (const T & t1, const T & t2)
{
   return t1 == t2;
}

template <>
void per_type_init (zoidfs_dirent_t & d)
{
   std::generate (reinterpret_cast<char*>(&d),
         reinterpret_cast<char*>(&d) + sizeof(d),
         random);
   randomString (d.name, sizeof(d.name));
}

// ============================================================================
// ============ Testing Helper functions  =====================================
// ============================================================================

/**
 * Return the size under a given size processor
 */
template <typename T, typename SIZE>
Size::SizeInfo getSize (SIZE & size, const T & type)
{
   SizeSaver<SIZE> restore (size);

   size.reset ();
   process (size, type);
   return size.size();
}

/**
 * Return the size under a given size processor
 */
template <typename T, typename SIZE>
Size::SizeInfo getSize (SIZE & size)
{
   const T dummy = T();
   return getSize (size, dummy);
}


/**
 * Check if the size of the type is as expected
 */
template <typename T, typename SIZE>
void validateSize (SIZE & size, size_t expected)
{
   const size_t actual = getSize<T> (size).getActualSize();
   static boost::format f ("Checking if size of %s is %u as specified by the XDR spec... ");

   BOOST_TEST_MESSAGE(str(f % getName<T>() % expected));
   BOOST_CHECK_EQUAL(actual,expected);
}

/**
 * Validates if for a type T, the encoded size is in the range predicted
 * by WRITER and if the READER consumes the correct number of bytes.
 */
template <typename T, typename SIZE, typename WRITER, typename READER>
void validateSizeProcessor (SIZE & size, WRITER & writer, READER & reader)
{
   T dummy = T();
   static char buf[4096];
   reader.reset (&buf[0], sizeof(buf));
   writer.reset (&buf[0], sizeof(buf));

   process (writer, dummy);
   process (reader, dummy);
   const size_t writersize = writer.getPos ();
   const size_t readersize = reader.getPos ();
   const Size::SizeInfo predictedsize = getSize<T>(size);

   // Check if the size calculator predicted the correct range.
   BOOST_TEST_MESSAGE(str(boost::format("Checking if size for type %s is"
               " consistent") % getName<T>()));
   BOOST_CHECK (writersize >= predictedsize.getActualSize () &&
         writersize <= predictedsize.getMaxSize ());
   // Check if the reader and writer agree on the size
   BOOST_CHECK_EQUAL (writersize, readersize);
}

template <typename SIZE, typename WRITER, typename READER>
struct ValidateSizeProcessor
{
   ValidateSizeProcessor (SIZE & s, WRITER & w, READER & r)
      : size(s), writer(w), reader(r)
   {
   }

   template <typename T>
   void operator () (const T & )
   {
      validateSizeProcessor<T> (size, writer, reader);
   }

   SIZE & size;
   WRITER & writer;
   READER & reader;
};

/**
 * Validates if for a type T, the encoded value and the decoded value agree.
 */
template <typename T, typename SIZE, typename WRITER, typename READER>
void validateEncodeDecode (SIZE & , WRITER & writer, READER & reader)
{
   T dummy1;
   T dummy2;

   per_type_init (dummy1);
   per_type_init (dummy2);

   static char buf[4096];
   reader.reset (&buf[0], sizeof(buf));
   writer.reset (&buf[0], sizeof(buf));

   // Note: at some point we might need to add a 'flush' to the writer
   writer << dummy1;
   reader >> dummy2;

   static boost::format f ("Checking if type '%s' can be encoded and decoded"
        " correctly");
   BOOST_CHECK_MESSAGE (per_type_compare (dummy1, dummy2), str(f % getName<T>()));
}

template <typename SIZE, typename WRITER, typename READER>
struct ValidateEncodeDecode
{
   ValidateEncodeDecode (SIZE & s, WRITER & w, READER & r)
      : size(s),writer(w),reader(r)
   {
   }

   template <typename T>
   void operator () (const T & )
   {
      validateEncodeDecode<T> (size, writer, reader);
   }

   SIZE & size;
   WRITER & writer;
   READER & reader;
};

/**
 * Validate if the encoder/decoder detects buffer overflow/underflow.
 */
template <typename SIZE, typename ENCODER, typename DECODER>
void checkBufferProtection (SIZE & size, ENCODER & encoder, DECODER & decoder)
{
   uint8_t dummy;

   const Size::SizeInfo s (getSize(size, dummy));

   // Allocate buffer for one item
   char buf[s.getMaxSize()];

   encoder.reset (&buf[0], sizeof(buf));
   decoder.reset (&buf[0], sizeof(buf));

   // this should work
   BOOST_CHECK_NO_THROW (process (encoder, dummy));

   // this should not
   BOOST_CHECK_THROW (process (encoder, dummy), EncoderException);

   BOOST_CHECK_NO_THROW (process (decoder, dummy));
   BOOST_CHECK_THROW (process (decoder, dummy), EncoderException);
}

template <typename SIZE, typename ENCODER, typename DECODER>
void testString (SIZE & size, ENCODER & encoder, DECODER & decoder)
{
   const size_t MAXLEN = 512;
   boost::scoped_array<char> buf1 (new char[MAXLEN]);
   boost::scoped_array<char> buf2 (new char[MAXLEN]);

   randomString (&buf1[0], MAXLEN);

   EncString str1 (&buf1[0], MAXLEN);
   EncString str2 (&buf2[0], MAXLEN);

   size_t encsize = getSize (size, str1).getMaxSize();
   boost::scoped_array<char> encbuf (new char[encsize]);
   encoder.reset (encbuf.get(), encsize);
   process (encoder, str1);
   decoder.reset (encbuf.get(), encsize);
   process (decoder, str2);

   BOOST_CHECK(std::equal (&buf1[0], &buf1[0]+strlen(&buf1[0]),
            &buf2[0]));
}
 
//____________________________________________________________________________//
//____________________________________________________________________________//
//____________________________________________________________________________//
//____________________________________________________________________________//

BOOST_AUTO_TEST_SUITE( encoder )

struct F {
};

//____________________________________________________________________________//
// Specific test for XDR predefined sizes
//____________________________________________________________________________//

BOOST_FIXTURE_TEST_CASE( xdrsizes, F )
{
   BOOST_TEST_MESSAGE("Checking if the XDRSizeProcessor returns correct sizes"
         " for primitive XDR types");

   XDRSizeProcessor size;

   validateSize<uint8_t> (size, 4);
   validateSize<uint16_t> (size, 4);
   validateSize<uint32_t> (size, 4);
   validateSize<uint64_t> (size, 8);

   validateSize<int8_t> (size, 4);
   validateSize<int16_t> (size, 4);
   validateSize<int32_t> (size, 4);
   validateSize<int64_t> (size, 8);
}

//____________________________________________________________________________//



// ========================== XDR ===========================

BOOST_FIXTURE_TEST_CASE( correctsizes_xdr, F)
{
   BOOST_TEST_MESSAGE("Testing if XDR size prediction matches actual encoding");

   XDRSizeProcessor size;
   XDRWriter writer;
   XDRReader reader;

   ValidateSizeProcessor<XDRSizeProcessor,XDRWriter,XDRReader>
        proc (size, writer, reader);

   doGenericTests (proc);
}

BOOST_FIXTURE_TEST_CASE(correctserdeser_xdr,F)
{
   BOOST_TEST_MESSAGE("Testing if XDR decoding matches encoding");

   XDRSizeProcessor size;
   XDRWriter writer;
   XDRReader reader;

   ValidateEncodeDecode<XDRSizeProcessor,XDRWriter,XDRReader> proc 
      (size, writer, reader);
   doGenericTests (proc);
}

// --------------------------------------------------------------------------

// ========================== NONE ===========================

BOOST_FIXTURE_TEST_CASE( correctsizes_none, F)
{
   BOOST_TEST_MESSAGE("Testing if NONE size prediction matches actual encoding");

   encoder::none::Size   size;
   encoder::none::Writer writer;
   encoder::none::Reader reader;

   ValidateSizeProcessor<encoder::none::Size,
      encoder::none::Writer,
      encoder::none::Reader>
        proc (size, writer, reader);

   doGenericTests (proc);
}

BOOST_FIXTURE_TEST_CASE(correctserdeser_none,F)
{
   BOOST_TEST_MESSAGE("Testing if NONE decoding matches encoding");

   encoder::none::Size   size;
   encoder::none::Writer writer;
   encoder::none::Reader reader;

   ValidateSizeProcessor<encoder::none::Size,
      encoder::none::Writer,
      encoder::none::Reader>
        proc (size, writer, reader);

   doGenericTests (proc);
}


BOOST_AUTO_TEST_SUITE_END()


// EOF
