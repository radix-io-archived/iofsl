#include "ZeroCopyTransformInput.hh"
namespace iofwdevent {
/**
 * Constructor for ZeroCopyTransformInput.
 * @param[in] in        Transformed input stream which needs to be 
 *                      transformed back into its original stream.
 * @param[in] transform Transform to use on input data (in)
 */
ZeroCopyTransformInput::ZeroCopyTransformInput  (ZeroCopyInputStream * in, 
                                                 GenericTransform * transform) 
{
  this->mem = in;
  this->transform = transform;
}


/**
 * Read from the input stream and preform the transform from 
 * transformed input -> output.
 * @param[out] out       Pointer to location where output is going to be 
 *                       stored.
 * @param[out] len       size of out (actual)
 * @param[in]  cb        Callback to call when operation is complete 
 * @param[in]  suggested Size the client would like to see returned when 
 *                       read operation is completed. This is a best
 *                       effort and the actual size may vary from 
 *                       suggested.
 */
Handle ZeroCopyTransformInput::read (const void ** out, size_t * len, 
                                     const CBType & cb, size_t suggested)
{
  assert(1 == 0);
}

/**
 * Return undigested input back to the stream 
 * @param[in] len        Size of data that is being returned to the stream
 * @param[in] cb         Callback when operation is complete.
 */
Handle ZeroCopyTransformInput::rewindInput (size_t len, const CBType & cb)
{
  assert(1 == 0);
}
}

