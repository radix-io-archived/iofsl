#ifndef IOFWDUTIL_TRANSFORM_IOSTATE_HH
#define IOFWDUTIL_TRANSFORM_IOSTATE_HH

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
       */
      class IOWriteBuffer
      {
            public:

               IOWriteBuffer (size_t maxaccess)
                  : maxaccess_(maxaccess), extbuf_(0), extbuf_size_(0),
                  intbuf_(new char[maxaccess]),
                  intbuf_used_(0), intbuf_size_(maxaccess),
                  intbuf_consumed_(0), lastsegsize_(0)
               {
               }

               /**
                * Return pointer to region of size bytes.
                * Returns 0 if no buffer space is available.
                */
               void * reserve (size_t bytes);

               /**
                * Returns true if all the bytes could be released.
                * If false, updateWrite needs to be called to provide more
                * output space and the call to releaseWrite should be repeated
                * _using the same value for bytes_.
                *
                * Note that it is valid /for writes/ to release less than was
                * reserved, in which case the remaining bytes in the
                * reservation will be discarded.
                */
               bool release (size_t bytes);

               /**
                * Provide a new buffer to write to
                */
               void update (void * ext, size_t extsize);

               /**
                * Reset internal state. Max access size remains unchanged
                * between resets. Unreleased data is discarded.
                */
               void reset ();

               /**
                * Return how many bytes were written to the external buffer.
                * Note: this does not include any data that might be buffered
                * internally; To get a correct value, call this only after
                * calling release (and before calling reserve).
                */
               size_t getWritten () const;


            protected:
               const size_t maxaccess_;

               char * extbuf_;
               size_t extbuf_size_;
               size_t extbuf_used_;

               boost::scoped_array<char> intbuf_;
               size_t intbuf_used_;
               const size_t intbuf_size_;

               // For write mode: how much of the internal buffer was copied
               // to the external buffer
               size_t intbuf_consumed_;

               // for debugging
               size_t lastsegsize_;
         };


      //=====================================================================
   }
}
#endif
