/**
 * @class ZeroCopyMemoryInput
 * 
 * @brief Zero copy input stream implementation for creating a stream from a
 *        region in memory.
 *
 * This class allows for a ZeroCopyInputStream to be created using a region
 * of memory. This allows for this region of memory to be accessed using
 * the ZeroCopyInputStream interfacing style.
 *
 */
#include "ZeroCopyInputStream.hh"
#include "CBType.hh"
#include "Handle.hh"
#include "CBException.hh"
#include "iofwdutil/tools.hh"
#include <boost/smart_ptr.hpp>

using namespace boost;
namespace iofwdevent {
  /**
   * Creates an input zero copy stream from a memory region 
   */
  class ZeroCopyMemoryInput: public ZeroCopyInputStream {
    protected:
      boost::scoped_ptr <char> mem; /*< Stores memory location data */
      size_t memSize;       /*< Stores the size of the memory location */
      size_t pos;           /*< Stores current pointer position inside mem */

    public:
      /* Constructor for ZeroCopyMemoryInput. */
      ZeroCopyMemoryInput  (void ** , size_t );

      /* Cancel operation (not used since this class does not block) */
      void cancel (Handle UNUSED(x)) {}; 

      /* Read from the input stream */
      Handle read (const void **, size_t * , const CBType &, size_t );

      /* Rewind the input stream */
      Handle rewindInput (size_t , const CBType & );
      
  };
}


