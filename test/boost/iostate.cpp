#include <boost/format.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/bind.hpp>
#include <boost/foreach.hpp>
#include <boost/scoped_ptr.hpp>
#include <vector>
#include <numeric>

#include "iofwdutil/assert.hh"
#include "iofwdutil/transform/IOReadBuffer.hh"
#include "iofwdutil/transform/IOWriteBuffer.hh"
#include "test_common.hh"
#include "TestData.hh"
#include "MultiAlloc.hh"

using namespace iofwdutil::transform;
using namespace boost;


enum {    MAX_ACCESS = 1024*64,    // Max size of request to IOState
          LOOPS = 6,              // Number of loops in read/write test
          SEED = 0x6666,           // Random seed
          MAX_BUFS = 64,          // Max number of buffers in sequence
          MAX_BUFSIZE = 256*1024  // Max size of single buffer in sequence
   };


struct Fixture
{
   public:

   Fixture ()
      : iostate_read_ (MAX_ACCESS),
        iostate_write_ (MAX_ACCESS)
   {
      srand (SEED);
   }

   void freeMem ()
   {
      mem_.reset ();
   }


   void clearMem ()
   {
      processMem (zeroMem);
   }

   template <typename FUNC>
   void processMem (FUNC f)
   {
      const size_t memcount = mem_->size();
      void ** mem = mem_->getMem ();
      size_t * memsizes = mem_->getMemSizes();
      for (size_t i=0; i<memcount; ++i)
      {
         f (mem[i], memsizes[i]);
      }
   }

   void allocateMem ()
   {
      freeMem ();
      mem_.reset (new MultiAlloc (MAX_BUFS, MAX_BUFSIZE));
   }

   ~Fixture ()
   {
   }

   static void zeroMem (void * ptr, size_t bytes)
   {
      memset (ptr, 0, bytes);
   }

   template <typename FUNC>
   void iostateDriver (bool read, FUNC f)
   {
      if (read)
         iostate_read_.reset();
      else
         iostate_write_.reset();

      if (mem_->empty())
      {
         BOOST_TEST_MESSAGE("Mem list is empty. Skipping test");
         return;
      }

      const size_t * memsizes = mem_->getMemSizes();
      void ** mem = mem_->getMem ();
      const size_t memcount = mem_->size();

      size_t bytesremaining = std::accumulate (memsizes, memsizes + memcount,
            0);

      size_t curbuf = 0;

      if (read)
         iostate_read_.update (mem[curbuf], memsizes[curbuf]);
      else
         iostate_write_.update (mem[curbuf], memsizes[curbuf]);

      ++curbuf;

      while (bytesremaining)
      {
         const size_t segsize = std::min (bytesremaining,
               static_cast<size_t>((random () % MAX_ACCESS)+1));

         void * ptr = 0;
         
         while (!ptr)
         {
            ptr = (read ?
                  const_cast<void*>(iostate_read_.reserve(segsize))
                  : iostate_write_.reserve (segsize));

            if (!ptr)
            {
               ALWAYS_ASSERT(curbuf < memcount);
               if (read)
                  iostate_read_.update (mem[curbuf], memsizes[curbuf]);
               else
                  iostate_write_.update (mem[curbuf], memsizes[curbuf]);
               ++curbuf;
            }
         }

         f (ptr, segsize);

         if (read)
         {
            // read is always able to release
            iostate_read_.release (segsize);
         }
         else
         {
            while (true)
            {
               const bool released = iostate_write_.release (segsize);
               if (!released)
               {
                  // Ran out of buffer space; provide more space
                  ALWAYS_ASSERT(curbuf < memcount);
                  if (read)
                     iostate_read_.update (mem[curbuf], memsizes[curbuf]);
                  else
                     iostate_write_.update (mem[curbuf], memsizes[curbuf]);
                  ++curbuf;
               }
               else
               {
                  break;
               }
            }
         }

         ASSERT(bytesremaining >= segsize);
         bytesremaining -= segsize;
      }
   }

   IOReadBuffer iostate_read_;
   IOWriteBuffer iostate_write_;

   TestData validator_;
   TestData generator_;

   boost::scoped_ptr<MultiAlloc> mem_;

};



BOOST_FIXTURE_TEST_CASE ( iostate_read1, Fixture )
{
   BOOST_TEST_MESSAGE("Testing read IOState");

   const size_t loops = (isLongTestEnabled() ? 30*LOOPS : LOOPS);
   for (size_t i=0; i<loops; ++i)
   {
      generator_.reset ();
      validator_.reset ();

      // First iteration, we only want a single segment
      allocateMem ();

      BOOST_TEST_MESSAGE(str(format("iter %i: %i segments")
            % (i+1) % mem_->size()));

      // fill data with pattern
      processMem (boost::bind (&TestData::generate, boost::ref(generator_),
               _1, _2));

      iostateDriver (true,
            boost::bind (&TestData::validate, boost::ref(validator_), _1, _2));

      freeMem ();
   }
}

BOOST_FIXTURE_TEST_CASE (iostate_write, Fixture)
{
   BOOST_TEST_MESSAGE("Testing write IOState");

   const size_t loops = (isLongTestEnabled() ? 30*LOOPS : LOOPS);

   for (size_t i=0; i<loops; ++i)
   {
      generator_.reset ();
      validator_.reset ();

      // First iteration, we only want a single segment
      allocateMem ();

      // Initialize mem to 0
      clearMem ();

      BOOST_TEST_MESSAGE(str(format("iter %i: %i segments")
            % (i+1) % mem_->size()));

      iostateDriver (false,
            boost::bind (&TestData::generate, boost::ref(generator_), _1,
               _2));

      // Check if correct output pattern was written to mem
      processMem (boost::bind (&TestData::validate, boost::ref(validator_),
               _1, _2));

      freeMem ();
   }
}
