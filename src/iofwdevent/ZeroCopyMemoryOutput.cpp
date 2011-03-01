#include "ZeroCopyMemoryOutput.hh"
#include "ZeroCopyMemoryInput.hh"
using namespace boost;
namespace iofwdevent {

/**
 * Constructor for ZeroCopyMemoryOutput.
 * @param[in] in        Memory region to serve as the output location for the
 *                      stream (this memory must be allocated externally!)
 * @param[in] len       Length of the memory region void * out.
 */
ZeroCopyMemoryOutput::ZeroCopyMemoryOutput (void * output, size_t len)
{
  this->mem = output;
  this->pos = 0;
  this->memSize = len;
  this->offset = 0;
}

void ZeroCopyMemoryOutput::reset()
{
  this->pos = 0;
  this->offset = 0;
}

void ZeroCopyMemoryOutput::setOffset(size_t s)
{
  this->offset = s;
}

void * ZeroCopyMemoryOutput::getMemPtr()
{ 
  return this->mem;
}

size_t ZeroCopyMemoryOutput::getOffset()
{
  return this->offset;
}

size_t ZeroCopyMemoryOutput::getTotalLen()
{
  return this->memSize;
}
/**
 * Converts a ZeroCopyMemoryInput stream to a ZeroCopyOutputStream. Consider
 * the input stream (ZeroCopyInputStream * in) to be invalid after this 
 * conversion takes place.
 * @param[in] in Memory Input stream to convert. Reset will be called on this 
 *               Stream at the end of this function call.
 */
void ZeroCopyMemoryOutput::convertToOutput ( ZeroCopyMemoryInput * in)
{
  mem = in->getMemPtr();
  offset = in->getOffset();
  memSize = in->getTotalLen();
}

/**
 * Write to the stream returning a pointer to the region of memory where 
 * the data can be written to. 
 * @param[out] ptr      Location where a pointer containing a region of memory
 *                      can be written to is stored. This pointer is where you
 *                      can safely write data. 
 * @param[out] size     Number of bytes that can be written to ptr.
 * @param[in]  cb       Callback function to be used when operation is completed
 * @param[in] suggested Client specified suggested write size. This write size 
 *                      is best effort and the actual write size may be more or
 *                      less then the number specified by this parameter 
 * @return              Returns a handle relating to this particular write. 
 *                      (not used with ZeroCopyMemoryOutput due to no blocking
 *                      in this class)
 */
Handle ZeroCopyMemoryOutput::write (void ** ptr, size_t * size, 
                                    const CBType & cb, size_t suggested)
{
  if (suggested > this->memSize - this->pos)
  {
    (*ptr) = (char *)this->mem + (this->pos);
    (*size) = this->memSize - this->pos;
    this->pos = this->memSize;
  }
  else 
  {
    (*ptr) = (char *)this->mem + (this->pos);
    (*size) = suggested;
    this->pos = this->pos + suggested;
  }
  /* Call the callback */
  cb(*(new CBException()));
  /* Do not return a handle with this call (non-blocking) */
  return (void *)0;  
}

/**
 * Allows for the stream to be "rewinded" and unused portions of a write to be
 * returned to ZeroCopyMemoryOutput. 
 * @param[in] size      How much to rewind.
 * @param[in] cb        Callback to use when the operation is completed (called 
 *                      immedietly).
 * @return              Returns a handle relating to this particular rewind. 
 *                      (not used with ZeroCopyMemoryOutput due to no blocking
 *                      in this class)
 */
Handle ZeroCopyMemoryOutput::rewindOutput (size_t size, const CBType & cb)
{
  if (size - this->pos >= this->offset)
    this->pos = this->pos - size;

  cb(*(new CBException()));
  return (void *)0; 
}

/**
 * Returns the amount of buffer remaining for the write
 * @return              Value containing remaining space left in buffer 
 *                      (this->mem)
 */
size_t ZeroCopyMemoryOutput::spaceRemaining (void)
{
  return this->memSize - this->pos;
}

/**
 * Flush the internal buffer (permanently advance the pointer).
 * @param[in] cb        Callback to use when the operation is completed (called 
 *                      immedietly).
 * @return              Returns a handle relating to this particular flush. 
 *                      (not used with ZeroCopyMemoryOutput due to no 
 *                      blocking in this class)
 */
Handle ZeroCopyMemoryOutput::flush (const CBType & cb)
{
  this->offset = this->pos;
  cb(*(new CBException()));
  return (void *)0; 
}

}
