#include <boost/format.hpp>

#include "iofwdutil/LinkHelper.hh"
#include "iofwdutil/transform/IOFWDZLib.hh"
#include "TransformException.hh"

using boost::format;


GENERIC_FACTORY_CLIENT(std::string,
      iofwdutil::transform::GenericTransform,
      iofwdutil::transform::ZLib,
      "ZLIB",
      zlib);


namespace iofwdutil
{

  namespace transform
  {

    ZLib::ZLib()
    {
      int ret = 0;

      stream.zalloc = Z_NULL;
      stream.zfree = Z_NULL;
      stream.opaque = Z_NULL;
      stream.avail_in = 0;
      stream.next_in = Z_NULL;

      decompress_state = SUPPLY_INBUF;
      outbuf_partially_filled = false;

      ret = inflateInit(&stream);
      if(Z_OK != ret) {
        throw TransformException (str(format("inflateInit() returned error"
                   " %d") % ret));
      }
    }


    ZLib::~ZLib()
    {
      inflateEnd(&stream);
    }


    state ZLib::getTransformState () const
    {
      return decompress_state;
    }

    void ZLib::transform(const void *const inBuf,
                         size_t inSize,
                         void *outBuf,
                         size_t outSize,
                         size_t *outBytes,
                         int *outState,
                         bool flushFlag)
    {
      int flag = (flushFlag == true) ? Z_SYNC_FLUSH : Z_NO_FLUSH;
      size_t have = 0;
      int ret = 0;

      if(SUPPLY_INBUF == decompress_state)
      {
        stream.avail_in = inSize;
        if(0 == stream.avail_in) {
          *outState = decompress_state = SUPPLY_INBUF;
          return;
        }
        stream.next_in = (Bytef*)inBuf;
      }

      if(false == outbuf_partially_filled) {
        stream.avail_out = outSize;
        stream.next_out = (Bytef*)outBuf;
      }

      ret = inflate(&stream, flag);

      const char * err;
      switch(ret)
      {
        case Z_STREAM_ERROR:
           err = "stream error";
           break;
        case Z_NEED_DICT:
           err = "need dict"; break;
        case Z_DATA_ERROR:
           err = "data error"; break;
        case Z_MEM_ERROR:
           err = "mem error"; break;
        default:
           err = 0;
      }

      if (err)
      {
          inflateEnd(&stream);
          throw TransformException (err);
      }

      *outBytes = have = outSize - stream.avail_out;

      if(have == outSize)
      {
        outbuf_partially_filled = false;
        decompress_state = CONSUME_OUTBUF;
      } 
      else if(have == 0)
      {
        outbuf_partially_filled = false;
        decompress_state = SUPPLY_INBUF;
      }
      else
      {
        outbuf_partially_filled = true;
        decompress_state = SUPPLY_INBUF;
      }

      if(Z_STREAM_END == ret)
      {
        decompress_state = GENERIC_TRANSFORM_LAST;
      }

      *outState = decompress_state;

    }

  }

}
