#include <string>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include "ZLogSource.hh"
#include "ZLogDefaultFilter.hh"

using namespace boost;
using namespace boost::gregorian;
using namespace boost::posix_time;

namespace iofwdutil
{
   namespace zlog
   {
//===========================================================================


ZLogDefaultFilter::ZLogDefaultFilter ()
   : fmt_("[%s/%u] T%i (%s) %s\n")
{
}

ZLogDefaultFilter::~ZLogDefaultFilter ()
{
}

void ZLogDefaultFilter::initialize ()
{
}

void ZLogDefaultFilter::setOption (const std::string & name,
      const std::string & value)
{
   boost::mutex::scoped_lock l (lock_);
   ZLogFilter::setOption (name, value);
}

std::string ZLogDefaultFilter::getTime () const
{
   return to_iso_string(microsec_clock::local_time());
}

void ZLogDefaultFilter::filterMsg (const ZLogSource & source,
      int level, std::string & msg)
{
   // fmt is shared: need to lock
   boost::mutex::scoped_lock l (lock_);
   msg = str(fmt_ % getTime() % level % boost::this_thread::get_id() %
         source.getSourceName () % msg);
}


//===========================================================================
   }
}
