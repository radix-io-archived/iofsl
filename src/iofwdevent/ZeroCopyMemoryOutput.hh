/**
 * @class ZeroCopyMemoryOutput
 * 
 * @brief Zero copy output stream implementation for creating a stream from a
 *        region in memory.
 *
 * This class allows for a ZeroCopyOutputStream to be created using a region
 * of memory. This allows for this region of memory to be accessed using
 * the ZeroCopyOutputStream interfacing style.
 *
 */
#include "ZeroCopyOutputStream.hh"
#include "CBType.hh"
#include "Handle.hh"
#include "CBException.hh"
#include "iofwdutil/tools.hh"
#include <boost/smart_ptr.hpp>

using namespace boost;
namespace iofwdevent {
  class ZeroCopyMemoryOutput: public ZeroCopyOutputStream {
    protected:
      boost::scoped_ptr <char> mem; /*< Stores memory location data */
      size_t memSize;       /*< Stores the size of the memory location */
      size_t pos;           /*< Stores current pointer position inside mem */
      size_t offset;        /*< Minimum possible memory position for M */
      void ** output;       /*< Where a write will be flushed to */
    public:
      /* Constructor for InputMemoryZeroCopy. */
      ZeroCopyMemoryOutput  (void ** ,size_t );

      /* Cancel operation (not used since this class does not block) */
      void cancel (Handle UNUSED(x)) {}; 

      /* Returns pointer to memory area where a write can take place */
      Handle write (void **, size_t *, const CBType & , size_t);

      /* Rewinds the write stream */
      Handle rewindOutput (size_t, const CBType &);

      /* Flush internal buffers, if any */
      Handle flush (const CBType & );

  };
}


