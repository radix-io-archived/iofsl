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
#ifndef SRC_IOFWDEVENT_ZEROCOPYMEMORYINPUT
#define SRC_IOFWDEVENT_ZEROCOPYMEMORYINPUT
#include "ZeroCopyInputStream.hh"
#include "CBType.hh"
#include "Handle.hh"
#include "CBException.hh"
#include <boost/smart_ptr.hpp>

using namespace boost;
namespace iofwdevent {
  /**
   * Creates an input zero copy stream from a memory region 
   */
  class ZeroCopyMemoryInput : public ZeroCopyInputStream {
    protected:
      const void * mem; /*< Stores memory location data */
      size_t memSize;       /*< Stores the size of the memory location */
      size_t pos;           /*< Stores current pointer position inside mem */

    public:
      /* Constructor for ZeroCopyMemoryInput. */
      ZeroCopyMemoryInput  (const void * , size_t );

      /* Cancel operation (not used since this class does not block) */
      void cancel (Handle x) {x = (Handle)0;}; 

      /* Read from the input stream */
      Handle read (const void **, size_t * , const CBType &, size_t );

      /* Rewind the input stream */
      Handle rewindInput (size_t , const CBType & );

      /* Resets the ZeroCopyMemoryInput stream and sets it to a new memory 
         location */
      int reset (const void *, size_t);

      /* set the position offset for last read */
      void setOffset(size_t);

      /* Amount of space remaining that has not be read in this->mem */
      size_t spaceRemaining (void);
  };
}
#endif

