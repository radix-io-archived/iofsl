#include "ZeroCopyMemoryInput.hh"
using namespace boost;
namespace iofwdevent {

/**
 * Constructor for ZeroCopyMemoryInput.
 * @param[in] in Memory region to use for the Input Stream
 * @param[in] len Length of the memory region void * in.
 */
ZeroCopyMemoryInput::ZeroCopyMemoryInput (void ** in, size_t len)
{
  this->mem.reset((char *)(*in));
  this->pos = 0;
  this->memSize = len;
}

/**
 * Read's from the stream returning a pointer to the region of memory where 
 * the data is stored. 
 * @param[out] ptr void ** pointer containing the address to the data that was 
 *                 read
 * @param[out] size Number of bytes that were read.
 * @param[in] cb Callback function to be used when operation is completed
 * @param[in] suggested Client specified suggested read size. This read size 
 *                      is best effort and the actual read may be more or less
 *                      then the value specified in suggested.
 * @return Returns a handle relating to this particular read. (not used with
 *         ZeroCopyMemoryInput due to no blocking in this class)
 */
Handle ZeroCopyMemoryInput::read (const void ** ptr, size_t * size, 
                                  const CBType & cb, size_t suggested)
{
  if (suggested > this->memSize - this->pos)
  {
    (*ptr) = this->mem.get() + (this->pos);
    (*size) = this->memSize - this->pos;
    this->pos = this->memSize;
  }
  else 
  {
    (*ptr) = this->mem.get() + (this->pos);
    (*size) = suggested;
    this->pos = this->pos + suggested;
  }
  /* Call the callback */
  cb(*(new CBException()));
  /* Do not return a handle with this call (non-blocking) */
  return (void *)0;  
}


/**
 * Allows for the stream to be "rewinded" and unused portions of a read to be
 * returned to ZeroCopyMemoryInput. This prevents data being skipped on 
 * subsequent reads.
 * @param[in] size How much to rewind.
 * @param[in] cb Callback to use when the operation is completed (called 
 *               immedietly).
 * @return Returns a handle relating to this particular rewind. (not used with
 *         ZeroCopyMemoryInput due to no blocking in this class)
 */
Handle ZeroCopyMemoryInput::rewindInput (size_t size, const CBType & cb)
{
  if (size - this->pos >= 0)
    this->pos = this->pos - size;
  cb(*(new CBException()));
  return (void *)0; 
}
}
