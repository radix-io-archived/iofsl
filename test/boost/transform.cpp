#include <memory>
#include <boost/format.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/function.hpp>
#include <boost/ptr_container/ptr_vector.hpp>

#include "iofwd_config.h"

#include "iofwdutil/assert.hh"
#include "iofwdutil/transform/LZF.hh"
#include "iofwdutil/transform/LZFCompress.hh"
#include "iofwdutil/transform/CopyTransform.hh"

// @TODO: use factory so this can go
#ifdef HAVE_ZLIB
#  include "iofwdutil/transform/ZLibCompress.hh"
#  include "iofwdutil/transform/ZLibDecompress.hh"
#  include "iofwdutil/transform/IOFWDZLib.hh"
#endif

#ifdef HAVE_BZLIB
#  include "iofwdutil/transform/BZCompress.hh"
#  include "iofwdutil/transform/BZDecompress.hh"
#endif

#include "iofwdutil/Timer.hh"
#include "test_common.hh"
#include "MultiAlloc.hh"
#include "TestData.hh"

using namespace iofwdutil::transform;
using namespace boost;


enum {
          LOOPS = 6,              // Number of loops in tests
          SEED = 0x6666,           // Random seed
          MAX_BUFS = 6,          // Max number of buffers in sequence
          MAX_BUFSIZE = 256*1024  // Max size of single buffer in sequence
   };

// ==========================================================================
// ==========================================================================

struct Dummy {};

static void nop (const void *, size_t )
{
}
typedef boost::function<void (void *, size_t)> Op;

static size_t driveTransform (
      size_t inputbytes,
      MultiAlloc & input,
      MultiAlloc & output,
      GenericTransform & transform,
      Op pre,
      Op post)
{
   void * const * in = input.getMem ();
   const size_t incount = input.size();
   const size_t * insize = input.getMemSizes ();

   void * outbuf = 0;
   size_t outsize;
   size_t totalwritten = 0;
   size_t inremaining = inputbytes;
   for (size_t i=0; i<incount; ++i)
   {
      void * data = in[i];
      const size_t datasize = std::min(insize[i], inremaining);

      if (!inremaining)
         break;

      inremaining -= datasize;


      // Run pre function on input buffer
      pre (data, datasize);

      while (true)
      {
         size_t written;
         int state;

         if (!outbuf)
         {
            boost::tie (outbuf, outsize) = output.addRandomBuf (MAX_BUFSIZE);
         }

         // we set flushflag if we're at the last input buffer
         const bool flush = (i+1 == incount);

         transform.transform (data, datasize, outbuf, outsize,
               &written, &state, flush);

         // If we set the flush flag, the transform should not ask for more
         // input data. It should either ask for more output space or indicate
         // that all is done.
         BOOST_CHECK (!flush || state != SUPPLY_INBUF);

         if (state == CONSUME_OUTBUF || state == TRANSFORM_DONE)
         {
            // have output buffer ready

            // We assume that the transform will use all out the output buffer
            // space we give it *before* asking for more.
            if (!flush)
            {
               BOOST_CHECK_EQUAL (written, outsize);
            }
            
            // Run post function on output of transform
            post (outbuf, written);
            totalwritten += written;
            outbuf = 0;

            if (written < outsize)
            {
               BOOST_CHECK_EQUAL (state, TRANSFORM_DONE);
            }

            if (state == TRANSFORM_DONE)
            {
               break;
            }
         }
         else if (state == SUPPLY_INBUF)
         {
            break;
         }
         else
         {
            BOOST_FAIL("Got unknown output state from transform!");
         }
      }
   }
   return totalwritten;
}


static void testTransform (GenericTransform & encode, GenericTransform &
      decode)
{
   TestData generator;
   TestData validator;

   MultiAlloc input_mem (MAX_BUFS, MAX_BUFSIZE);
   MultiAlloc output_mem;

   if (input_mem.empty())
   {
      // we don't want empty mem
      input_mem.addRandomBuf (MAX_BUFSIZE);
   }

   const size_t bufcount = input_mem.size();
   const size_t totalsize = input_mem.totalSize();

   BOOST_TEST_MESSAGE (format("Encoding %i buffers, total size %i") %
         bufcount % totalsize);
   
   encode.reset ();
   iofwdutil::Timer t;
   t.start ();
   const size_t encodedsize = driveTransform (totalsize, input_mem,
         output_mem,  encode,
         boost::bind (&TestData::generate, boost::ref(generator), _1, _2),
         nop);
   t.stop ();
   BOOST_TEST_MESSAGE (format("Generated+Encoded %i bytes to %i bytes; %f MB/s; "
            "ratio=%f pct (note: time includes generating)")
         % totalsize
         % encodedsize
         % (double(totalsize) / (t.elapsed().toDouble()*1024*1024))
         % ((double(encodedsize)/totalsize)*100) );

   BOOST_TEST_MESSAGE (format("Trying to decode data"));
  
   input_mem.swap (output_mem);

   output_mem.reset ();
   decode.reset ();

   t.reset ();
   t.start ();
   const size_t decodedsize = driveTransform (encodedsize, input_mem,
         output_mem, decode, nop,
         boost::bind (&TestData::validate, boost::ref(validator), _1, _2));
   t.stop ();

   BOOST_TEST_MESSAGE (format("Decoded (and validated) @ %f MB/s; ")
         % (double(totalsize) / (t.elapsed().toDouble()*1024*1024)));
   BOOST_REQUIRE_EQUAL (decodedsize, totalsize);


   BOOST_TEST_MESSAGE (format("Test completed"));
}


struct TestEntry
{
   TestEntry (const char * n, GenericTransform * e, GenericTransform * d)
      : name(n), encode(e), decode(d) {}

   virtual ~TestEntry ()
   {}

   const char * name;
   std::auto_ptr<GenericTransform> encode;
   std::auto_ptr<GenericTransform> decode;
};

// @TODO: once tag support is added to factory, find pairs of encode/decode
// transforms automatically and test them.
BOOST_FIXTURE_TEST_CASE ( transform_encode_decode, Dummy )
{
   BOOST_TEST_MESSAGE("Testing encoding and decoding");

   size_t loops = LOOPS * (isLongTestEnabled() ? 30 : 1);

   boost::ptr_vector<TestEntry>  tests;


   tests.push_back (new TestEntry("Copy",
       new iofwdutil::transform::CopyTransform(),
       new iofwdutil::transform::CopyTransform()));
   
    tests.push_back (new TestEntry("LZF",
         new iofwdutil::transform::LZFCompress (),
         new iofwdutil::transform::LZF ()));

#ifdef HAVE_ZLIB
   tests.push_back (new TestEntry("zlib-1",
         new iofwdutil::transform::ZLibCompress (),
         new iofwdutil::transform::ZLibDecompress ()));
   // disabled since reset is not supported
/*   tests.push_back (new TestEntry("zlib-2",
         new iofwdutil::transform::ZLibCompress (),
         new iofwdutil::transform::ZLib ()));
         */
#endif

#ifdef HAVE_BZLIB
   tests.push_back (new TestEntry("bzlib",
         new iofwdutil::transform::BZCompress (),
         new iofwdutil::transform::BZDecompress ()));
#endif

   for (size_t i=0; i<loops; ++i)
   {
      BOOST_FOREACH (const TestEntry & e, tests)
      {
      BOOST_TEST_MESSAGE(format("Testing %s") % e.name);
      testTransform (*e.encode, *e.decode);
      }
   }
}

