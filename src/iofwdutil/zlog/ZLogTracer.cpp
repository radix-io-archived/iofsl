#include <boost/format.hpp>

#include "ZLogTracer.hh"

namespace iofwdutil
{
   namespace zlog
   {
      //=====================================================================

      const std::string ZLogAutoTracer::getText (bool enter) const
      {
         const char * format1 = "-> %s (%s:%i)";
         const char * format2 = "<- %s (%s:%i)";
         return str(boost::format(enter ? format1 : format2)
               % funcname_ % filename_ % linenum_);
      }

      //=====================================================================
   }
}
