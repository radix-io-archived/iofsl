#include <boost/format.hpp>
#include "iofwdutil/tools.hh"
#include "ZLogException.hh"
#include "ZLogFilter.hh"

using namespace boost; 

namespace iofwdutil
{
   namespace zlog
   {
//===========================================================================

ZLogFilter::~ZLogFilter ()
{
}

void ZLogFilter::setOption (const std::string & name, const std::string &
      UNUSED(value))
{
   ZTHROW  (ZLogUnknownOptionException ()
         << zlog_option_name(name));
}
//===========================================================================
   }
}
