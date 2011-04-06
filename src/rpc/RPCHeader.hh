#ifndef RPC_RPCHEADER_HH
#define RPC_RPCHEADER_HH


#include "RPCKey.hh"

namespace rpc
{
   //========================================================================

   /**
    * This header is added to an outgoing RPC connection
    */
   struct RPCHeader
   {
      enum
      {
         FL_TRANSFORM_NONE  = 0x00000001,
         FL_TRANSFORM_ZLIB  = 0x00000002,
         FL_TRANSFORM_BZLIB = 0x00000004,
         FL_TRANSFORM_LZF   = 0x00000008,
         FL_TRANSFORM       = 0x000000ff,

         FL_CHECKSUM_NONE  = 0x00000100,
         FL_CHECKSUM_CRC32 = 0x00000200,
         FL_CHECKSUM_CRC64 = 0x00000400,
         FL_CHECKSUM_MD5   = 0x00000800,
         FL_CHECKSUM       = 0x0000ff00,

         FL_NOSTUFF        = 0x00010000,
         FL_OPTIONS        = 0x00ff0000,

         FL_DEFAULT        = FL_TRANSFORM_NONE | FL_CHECKSUM_NONE
      };

      RPCKey key;
      // Flags are relative to the side making the connection
      uint32_t flags_out;
      uint32_t flags_in;
   };

   // Ser/Deser function
   template <typename T>
   void process (T & tr, RPCHeader & h)
   {
      process (tr, h.key);
      process (tr, h.flags_out);
      process (tr, h.flags_in);
   }

   /**
    * This header is sent back with the RPC response
    */
   struct RPCResponse
   {
      enum {
         RPC_OK            = 0,
         RPC_LINKPARAM,         // Unknown checksum/compression
         RPC_UNKNOWN            // Other error
         // RPC_NO_PERM
      };

      uint32_t status;
   };

   template <typename T>
   void process (T & p, RPCResponse & r)
   {
      process (p, r.status);
   }

   //========================================================================
}

#endif
