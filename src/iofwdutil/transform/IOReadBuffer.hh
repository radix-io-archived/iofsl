#ifndef IOFWDUTIL_TRANSFORM_IOREADBUFFER_HH
#define IOFWDUTIL_TRANSFORM_IOREADBUFFER_HH

// for size_t
#include <cstring>
#include <boost/scoped_array.hpp>
#include "iofwdutil/assert.hh"

namespace iofwdutil
{
   namespace transform
   {
      //=====================================================================

      /**
       * This class makes certain assumptions:
       *   - no regions larger than max_access will be requested.
       *   - Release and reserve will used the same size
       */
      class IOReadBuffer
      {
            public:

               IOReadBuffer (size_t maxaccess)
                  : extbuf_(0), extbuf_size_(0),
                  maxaccess_(maxaccess), intbuf_(new char[maxaccess]),
                  intbuf_used_(0), intbuf_size_(maxaccess), lastsegsize_(0)
               {
               }

               /**
                * Returns a pointer to a region of size bytes (containing data
                * from input).
                * Returns 0 if no buffer space
                */
               const void * reserve (size_t bytes);

               /**
                * Release reserved buffer. Release should be called with the
                * same argument that was passed to reserve.
                */
               void release (size_t bytes);

               /**
                * Provide a new buffer to read from
                */
               void update (const void * ext, size_t extsize);

               /**
                * Reset internal state; update should be called after reset.
                */
               void reset ();

               /**
                * Return the amount of data we still have available in the
                * current buffer + internal buffer; i.e. the amount of
                * unprocessed data we can get before needing to call update.
                */
               size_t getAvail () const;

               /**
                * Returns true if there is no more data available in the
                * current buffer.
                */
               bool empty () const;

            protected:

               const char * extbuf_;
               size_t extbuf_size_;
               size_t extbuf_used_;
               const size_t maxaccess_;

               boost::scoped_array<char> intbuf_;
               size_t intbuf_used_;
               const size_t intbuf_size_;

               // for debugging
               size_t lastsegsize_;
         };


      //=====================================================================
   }
}
#endif
