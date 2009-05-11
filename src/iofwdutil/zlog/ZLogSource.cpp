#include <algorithm>
#include <boost/format.hpp>
#include "iofwdutil/assert.hh"
#include "zlog/ZLogSource.hh"


namespace iofwdutil
{
   namespace zlog
   {
//===========================================================================

ZLogSource::ZLogSource (const std::string & classname)
   : class_(classname), loglevel_ (INFO)
{
   // Initialize to zero 
   std::fill (sink_.begin(), sink_.end(), static_cast<ZLogSink*>(0)); 
}

void ZLogSource::doLog (unsigned int level, const boost::format & fmt)
{
   if (level <= loglevel_)
      this->doLog (level, str(fmt)); 
}

void ZLogSource::setSink (unsigned int level, ZLogSink * sink)
{
   ALWAYS_ASSERT (level < static_cast<unsigned int>(sink_.size ())); 
   sink_[level] = sink; 
}

void ZLogSource::addFilter (ZLogFilter * filter)
{
   ALWAYS_ASSERT (filter); 
   filters_.push_back (filter); 
}

//===========================================================================
   }
}
