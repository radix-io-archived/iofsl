#include "LZOCommon.hh"

#include "iofwdutil/assert.hh"
#include "iofwdutil/Singleton.hh"

#include <boost/format.hpp>

using boost::format;

namespace iofwdutil
{
   namespace transform
   {
      //=====================================================================

      namespace
      {

         struct LZOInit : public iofwdutil::Singleton<LZOInit>
         {
            LZOInit ();

         };

         LZOInit::LZOInit ()
         {
            LZOCommon::check (lzo_init ());
         }

      }

      LZOCommon::LZOCommon ()
      {
         // ensure we initialize LZO library
         LZOInit::instance ();
      }

      std::string LZOCommon::errorString (int ret)
      {
#define LZOERROR(a) case a: return #a
         switch (ret)
         {
            LZOERROR(LZO_E_OK);
            LZOERROR(LZO_E_INPUT_NOT_CONSUMED);
            LZOERROR(LZO_E_INPUT_OVERRUN);
            LZOERROR(LZO_E_OUTPUT_OVERRUN);
            LZOERROR(LZO_E_LOOKBEHIND_OVERRUN);
            LZOERROR(LZO_E_EOF_NOT_FOUND);
            LZOERROR(LZO_E_ERROR);
            default: return str(format("Unknown LZO error (%i)") % ret);
         }
#undef LZOERROR
      }


      //=====================================================================
   }
}
