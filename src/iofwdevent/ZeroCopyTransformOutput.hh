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

    class ZeroCopyTransformOutput : public ZeroCopyOutputStream {
      typedef iofwdutil::transform::GenericTransform GenericTransform;

      protected:
        ZeroCopyOutputStream * outStream; /*< Stores input stream from which to transform */
        GenericTransform * transform; /*< Stores transform information */
        ZeroCopyMemoryOutput * internalBuf; /* Stores the internal memory stream
                                             for this class */
        static const int SUPPLY_INBUF = iofwdutil::transform::SUPPLY_INBUF;
        static const int TRANSFORM_DONE = iofwdutil::transform::TRANSFORM_DONE;
        static const int CONSUME_OUTBUF = iofwdutil::transform::CONSUME_OUTBUF;
      public:
        ZeroCopyTransformOutput(ZeroCopyOutputStream * ,
                                GenericTransform * , size_t );
        Handle write ( void ** , size_t * , const CBType & , size_t );
        Handle getWriteLoc (void **, size_t *, const CBType & , size_t );
        void transformState (CBException ,void ** ,size_t * ,const CBType & ,
                        size_t , void ** , size_t * );
        int doTransform ( void ** ,  size_t * , bool );
        Handle flush (const CBType & );
        void flushTransform (const CBType &, void **, size_t *);
        Handle rewindOutput (size_t , const CBType & );
        void cancel (Handle x) { x = (Handle) 0; }; 
   
        void nullCB (CBException e) { e.check();};
    };
    //void blockingWriteCB(CBException );
}
