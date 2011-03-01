#include "ZeroCopyMemoryInput.hh"
#include "ZeroCopyMemoryOutput.hh"
using namespace boost;
namespace iofwdevent {

/**
 * Constructor for ZeroCopyMemoryInput.
 * @param[in] in        Region of memory (with data) to convert into an 
 *                      input stream
 * @param[in] len       Length of the memory region void * in.
 */
ZeroCopyMemoryInput::ZeroCopyMemoryInput (const void * in, size_t len)
{
  this->mem = in;
  this->pos = 0;
  this->memSize = len;
  this->totalSize = len;
}

size_t ZeroCopyMemoryInput::getTotalLen()
{
  return totalSize;
}

void * ZeroCopyMemoryInput::getMemPtr()
{
  return (void *) mem;
}

size_t ZeroCopyMemoryInput::getOffset()
{
  return pos;
}

void ZeroCopyMemoryInput::setOffset(size_t offset)
{
  pos = offset;
}

/** 
 * Converts a ZeroCopyMemoryOutput to a ZeroCopyMemoryInput (ONLY FLUSHED
 * OUTPUT WILL BE CONVERTED). 
 * @param[in] out ZeroCopyMemoryOutput stream to convert to input. This stream 
 *                should be considered invalid after this call has been made.
 *                Use ZeroCopyMemoryOutput::convertToOutput to change back.
 */
void ZeroCopyMemoryInput::convertToInput (ZeroCopyMemoryOutput * out)
{
  mem = out->getMemPtr();
  memSize = out->getOffset();
  totalSize = out->getTotalLen();
  out->reset();
}


/**
 * Read's from the stream returning a pointer to the region of memory where 
 * the data is stored. 
 * @param[out] ptr      void ** pointer containing the address to the data 
 *                      that was read
 * @param[out] size     Number of bytes that were read.
 * @param[in]  cb        Callback function to be used when operation is 
 *                      completed
 * @param[in]  suggested Client specified suggested read size. This read size 
 *                      is best effort and the actual read may be more or less
 *                      then the value specified in suggested.
 * @return              Returns a handle relating to this particular read. 
 *                      (not used with ZeroCopyMemoryInput due to no blocking 
 *                      in this class)         
 */
Handle ZeroCopyMemoryInput::read (const void ** ptr, size_t * size, 
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
 * Returns the amount of buffer remaining that has not been read 
 * @return              Value containing remaining space left in buffer 
 *                      (this->mem)
 */
size_t ZeroCopyMemoryInput::spaceRemaining (void)
{
  return this->memSize - this->pos;
}


/**
 * Resets the ZeroCopyMemoryInput stream and sets it to a new memory 
 * location
 * @param[in] in        Region of memory (with data) to convert into an 
 *                      input stream
 * @param[in] len       Length of the memory region void * in.
 * @return              Status of the operation ( > 0 = success)
 */
int ZeroCopyMemoryInput::reset (const void * in, size_t len)
{
  this->mem = in;
  this->pos = 0;
  this->memSize = len;
  return 1;
}


/**
 * Allows for the stream to be "rewinded" and unused portions of a read to be
 * returned to ZeroCopyMemoryInput. This prevents data being skipped on 
 * subsequent reads.
 * @param[in] size      How much to rewind.
 * @param[in] cb        Callback to use when the operation is completed (called 
 *                      immedietly).
 * @return              Returns a handle relating to this particular rewind. 
 *                      (not used with ZeroCopyMemoryInput due to no blocking 
 *                      in this class)         
 */
Handle ZeroCopyMemoryInput::rewindInput (size_t size, const CBType & cb)
{
  int64_t adjustPos = size;
  int64_t curPos = this->pos;
  if (adjustPos - curPos >= 0)
    this->pos = this->pos - size;
  cb(*(new CBException()));
  return (void *)0; 
}
}
