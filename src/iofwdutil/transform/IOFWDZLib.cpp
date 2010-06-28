#include "iofwdutil/transform/IOFWDZLib.hh"


namespace iofwdutil
{

  namespace iofwdtransform
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
	// Need to throw exception
	fprintf(stderr, "%s:(%s):%d inflateInit() returned error = %d\n", __FILE__, __func__, __LINE__, ret);
      }
    }


    ZLib::~ZLib()
    {
      inflateEnd(&stream);
    }


    state ZLib::getDecompressState()
    {
      return decompress_state;
    }

    void ZLib::transform(const void *const inBuf,
			 const size_t inSize,
			 void *outBuf,
			 const size_t outSize,
			 size_t *outBytes,
			 int *outState,
			 const bool flushFlag)
    {
      int flag = (flushFlag == true) ? Z_SYNC_FLUSH : Z_NO_FLUSH;
      int have = 0;
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

      switch(ret)
      {
	case Z_STREAM_ERROR:
	case Z_NEED_DICT:
	case Z_DATA_ERROR:
	case Z_MEM_ERROR:
	  inflateEnd(&stream);
	  *outState = decompress_state = TRANSFORM_STREAM_ERROR;
	  return;
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
	decompress_state = TRANSFORM_STREAM_END;
      }

      *outState = decompress_state;

    }

  }

}
