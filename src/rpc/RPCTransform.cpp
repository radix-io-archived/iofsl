#include "RPCTransform.hh"
#include "RPCHeader.hh"

#include "iofwdevent/ZeroCopyTransformInput.hh"
#include "iofwdevent/ZeroCopyTransformOutput.hh"

#include "iofwdutil/assert.hh"

using namespace iofwdutil::transform;
using namespace iofwdevent;

namespace rpc
{
   //========================================================================

   GenericTransform * getTransform (uint32_t flags, bool encode)
   {
      const char * name = 0;
      switch (flags & RPCHeader::FL_TRANSFORM)
      {
         case RPCHeader::FL_TRANSFORM_NONE:
            name = 0; break;
         case RPCHeader::FL_TRANSFORM_ZLIB:
            name = "ZLIB"; break;
         case RPCHeader::FL_TRANSFORM_BZLIB:
            name= "BZLIB"; break;
         case RPCHeader::FL_TRANSFORM_LZF:
            name = "LZF"; break;
         default:
            ALWAYS_ASSERT(false && "Unknown transform flag!");
      };
      if (!name)
         return 0;

      return encode ?
         GenericTransformEncodeFactory::instance().construct (name)() :
         GenericTransformDecodeFactory::instance().construct (name)();
   }

   ZeroCopyOutputStream * getOutputTransform (
         ZeroCopyOutputStream * out, uint32_t flags)
   {
      try
      {
         return flags & RPCHeader::FL_TRANSFORM_NONE ?
               out
             : new ZeroCopyTransformOutput (out, getTransform (flags, true), 0);
      }
      catch (...)
      {
         ALWAYS_ASSERT(false && "TODO: Filter proper nosuchtransform"
               " exception");
      }
      return 0;
   }

   ZeroCopyInputStream * getInputTransform (
         ZeroCopyInputStream * in, uint32_t flags)
   {
      try
      {
         return flags & RPCHeader::FL_TRANSFORM_NONE ?
            in
          : new ZeroCopyTransformInput (in, getTransform (flags, false), 0);
      }
      catch (...)
      {
         ALWAYS_ASSERT(false && "TODO: Filter proper nosuchtransform"
               " exception");
      }
      return 0;
   }


   //========================================================================
}

