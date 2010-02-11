#ifndef IOFWDUTIL_ZLOG_ZLOGCLASS_HH
#define IOFWDUTIL_ZLOG_ZLOGCLASS_HH

#include <string>
#include <vector>
#include <boost/array.hpp>
#include <boost/format/format_fwd.hpp>
#include <boost/thread.hpp>

#include "iofwdutil/zlog/ZLog.hh"
#include "iofwdutil/zlog/ZLogSink.hh"
#include "iofwdutil/zlog/ZLogFilter.hh"
#include "iofwdutil/assert.hh"

namespace iofwdutil
{
   namespace zlog
   {
//===========================================================================


class ZLogSource
{
public:
   ZLogSource (const std::string & classname);

   const std::string & getSourceName () const
   {
      return class_;
   }

   /// Returns true if logging for the specified level is enabled for
   /// this source.
   bool isEnabled (unsigned int level) const
   { return (level <= loglevel_); }

   void doLog (unsigned int level, const boost::format & fmt);

   void doLog (unsigned int level, const std::string & str)
   {
      boost::mutex::scoped_lock l(lock_);

      if (level > loglevel_)
         return;

      // maybe convert buf to TLS
      std::string buf (str);

      // Does not need locking: adding filters / changing sinks is not allowed
      // once logging starts.
      if (sink_[level] != 0)
      {
         for (unsigned int i=0; i<filters_.size(); ++i)
            filters_[i]->filterMsg (*this, level, buf);
         sink_[level]->acceptData (level,*this, buf);
      }
   }

   // Source does not take ownership of the sink
   void setSink (unsigned int level, ZLogSink * sink);

   // Source does not take ownership of the filter
   void addFilter (ZLogFilter * filter);

   // Set loglevel for this source
   void setLogLevel (unsigned int loglevel)
   {
      ALWAYS_ASSERT(loglevel < MAX_LEVEL);
      loglevel_ = loglevel;
   }

private:
   const std::string class_;

   boost::array<ZLogSink *, MAX_LEVEL> sink_;
   std::vector<ZLogFilter *> filters_;

   unsigned int loglevel_;

   boost::mutex lock_;
};


//===========================================================================
   }
}
#endif
