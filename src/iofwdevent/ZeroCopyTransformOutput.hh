/**
 * @class ZeroCopyTransformOutput
 * 
 * @brief Zero copy input stream implementation for creating a stream
 *        for converting to a transformed output (ex: compressed input).
 *
 * This class allows for a ZeroCopyOutputStream to be created an with data
 * being transformed by this stream
 */
#include "ZeroCopyOutputStream.hh"
#include "ZeroCopyInputStream.hh"
#include "ZeroCopyMemoryInput.hh"
#include "ZeroCopyMemoryOutput.hh"
#include "iofwdutil/transform/GenericTransform.hh"
#include "CBType.hh"
#include "Handle.hh"
#include "CBException.hh"
#include <deque>
#include <boost/smart_ptr.hpp>

using namespace iofwdutil;
using namespace boost;
namespace iofwdevent {
    class ZeroCopyTransformWriteWU {
      public:
        ZeroCopyTransformWriteWU(void ** ptr, size_t * size, 
                                 const CBType & cb, size_t suggested) :
            ptr_(ptr), size_(size), cb_(cb), suggested_(suggested)
        {
        }
        void ** ptr_;
        size_t * size_;
        const CBType cb_;
        size_t suggested_;
        ZeroCopyMemoryOutput * internalMem_;
    };

    class ZeroCopyTransformFlushWU {
      public:
        ZeroCopyTransformFlushWU (const CBType & cb) :
          cb_(cb)
        {
        }
        void * inData_;
        size_t inLen_;
        const CBType cb_;
    };

    class ZeroCopyTransformWU {
      public:
        ZeroCopyTransformWU (void * outData, size_t outLen, size_t outputSize,
                             int outState, bool flushFlag, const CBType & cb ) :
          outData_(outData), outLen_(outLen), outputSize_(outputSize), 
          outState_(outState), flushFlag_(flushFlag), cb_(cb)
        {
        }
        void * outData_;
        size_t outLen_;
        size_t outputSize_;
        int outState_;
        bool flushFlag_;
        const CBType cb_;
    };

    class ZeroCopyTransformOutput : public ZeroCopyOutputStream {
      typedef iofwdutil::transform::GenericTransform GenericTransform;
      typedef boost::function< void() > boundFunc;

      protected:
        ZeroCopyOutputStream * mem; /*< Stores input stream from which to transform */
        GenericTransform * transform; /*< Stores transform information */
        size_t memSize;          /*< Stores the size of the memory location */
        size_t pos;              /*< Stores current pointer position inside mem */
        ZeroCopyMemoryOutput * internalMem; /* Stores the internal memory stream
                                             for this class */
        char * internalMemPtr;
        mutable boost::mutex lock_;
        std::deque< boundFunc > writeCBQueue; 
        std::deque< boundFunc > flushCBQueue; 

      public:
        /* Constructor for InputMemoryZeroCopy. */
        ZeroCopyTransformOutput  (ZeroCopyOutputStream * , GenericTransform * , size_t);

        /* Cancel operation (not used since this class does not block) */
        void cancel (Handle x) { x = (Handle) 0;}; 

        /* Returns pointer to memory area where a write can take place */
        Handle write (void **, size_t *, const CBType & , size_t);

        /* Rewinds the write stream */
        Handle rewindOutput (size_t, const CBType &);

        /* Flush internal buffers, if any */
        Handle flush (const CBType & );

        /* Convert between a ZeroCopyMemoryOutput and a ZeroCopyMemoryInput */
        ZeroCopyMemoryInput * convertToMemoryInput(ZeroCopyMemoryOutput *);
        Handle blockingWrite (void ** , size_t * , const CBType &, size_t);
        void blockingWriteCB(CBException, ZeroCopyTransformWriteWU * );
        void nullCB (CBException );
        void doTransform (ZeroCopyTransformFlushWU * );
        void internalReadCB(CBException , ZeroCopyTransformFlushWU * );
    };
    //void blockingWriteCB(CBException );
}
