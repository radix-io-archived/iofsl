#ifndef RPC_RPCENCODER_HH
#define RPC_RPCENCODER_HH

#include "encoder/xdr/XDRSizeProcessor.hh"
#include "encoder/xdr/XDRWriter.hh"
#include "encoder/xdr/XDRReader.hh"
#include "encoder/EncoderException.hh"

namespace rpc
{
   //========================================================================

   /**
    * The following typedefs control how RPC messages are encoded/decoded.
    */
   typedef encoder::xdr::XDRWriter        RPCEncoder;
   typedef encoder::xdr::XDRReader        RPCDecoder;
   // typedef encoder::xdr::XDRSizeProcessor RPCSize;

   template <typename T>
   encoder::Size::SizeInfo getRPCEncodedSize (const T & t)
   {
      return encoder::xdr::getXDRSize (t);
   }


   //========================================================================
}

#endif
