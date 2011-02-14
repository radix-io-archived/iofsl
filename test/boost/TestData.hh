#ifndef TEST_BOOST_TESTDATA_HH
#define TEST_BOOST_TESTDATA_HH

#include <boost/format.hpp>
#include "iofwdutil/assert.hh"
#include "test/boost/DataValidator.hh"
#include "test/boost/DataGenerator.hh"

/**
 * Generates data (or validates it)
 */
struct TestData : public DataValidator,
                  public DataGenerator
{
   public:
      
      TestData ()
      {
         STATIC_ASSERT(sizeof(unsigned char) == 1);
         reset ();
      }

      void reset ()
      {
         nextval_ = 0x66;
         seen_ = 0;
      }

      void generate (void * ptr, size_t bytes)
      {
         unsigned char * p = static_cast<unsigned char *>(ptr);
         for (size_t i=0; i<bytes; ++i)
         {
            *p = nextval_;
            ++nextval_;
            ++seen_;
            ++p;
         }
       }

      void validate (const void * ptr, size_t bytes)
      {
         const unsigned char * p = static_cast<const unsigned char *>(ptr);
         for (size_t i=0; i<bytes; ++i)
         {
            if (*p != nextval_)
            {
               throw std::string (
                    str(boost::format("Was expecting %i, got %i. Aborting")
                        % (int) *p % (int) nextval_));
            }
            ++nextval_;
            ++seen_;
            ++p;
         }
      }

      size_t getSeen () const
      { return seen_; }

   protected:
      unsigned char nextval_;
      size_t seen_;
} ;



#endif
