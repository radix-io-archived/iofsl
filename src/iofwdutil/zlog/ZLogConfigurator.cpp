#include <boost/regex.hpp>
#include <boost/format.hpp>

#include "ZLogFilter.hh"
#include "ZLogSink.hh"
#include "ZLogSource.hh"
#include "ZLogConfigurator.hh"
#include "ZLogException.hh"
#include "iofwdutil/tools.hh"

using namespace boost; 

namespace iofwdutil
{
   namespace zlog
   {
//===========================================================================

std::string ZLogConfigurator::fixRegEx (const std::string & s)
{
   return s; 
}

ZLogConfigurator::MatchRule::MatchRule (const std::string & rx,
      bool final, ZLogSink * sink, const
      std::vector<ZLogFilter *> & filters)
   : final_(final), filters_(filters), sink_(sink)
{
   match_results<const char *> res;
   regex e ("\\s*([^:]*?)\\s*(?::\\s*(\\S+))?\\s*", regex::perl|regex::icase);

   if (!regex_match (rx.c_str(), res, e))
   {
      ZTHROW (ZLogException ()
            << zexception_msg(str(format("Invalid format for rule match: '%s'!") % rx)));
   }

   ALWAYS_ASSERT (res.size() == 2 || res.size() == 3); 
   std::string sourcematch (res[1]); 
   std::string levelmatch ((res.size() == 3 ? res[2] : std::string())); 

   if (!sourcematch.size())
      sourcematch = "*";
   
   // By default, all log levels will match
   levelmatch_.allowAll (); 

   sourcematch_.assign (fixRegEx (sourcematch)); 
   if (levelmatch.size())
   {
      levelmatch_.setup (levelmatch.c_str());
   }
}

bool ZLogConfigurator::MatchRule::matchSource (const std::string & name) const
{
   return regex_match (name, sourcematch_); 
}

bool ZLogConfigurator::MatchRule::matchLevel (int level) const
{
   return levelmatch_(level); 
}

//===========================================================================

ZLogConfigurator::ZLogConfigurator ()
{
}

ZLogConfigurator::~ZLogConfigurator ()
{
   clear (); 
}

void ZLogConfigurator::clear ()
{
   rules_.clear (); 
   for (FilterMap::iterator I=filters_.begin(); I!=filters_.end(); ++I)
   {
      delete (I->second); 
   }
   for (SinkMap::iterator I=sinks_.begin(); I!=sinks_.end(); ++I)
   {
      delete (I->second); 
   }
   filters_.clear(); 
   sinks_.clear (); 
}

bool ZLogConfigurator::configureSource (ZLogSource & source)
{
   const std::string sourcename (source.getSourceName ()); 

   bool matched = false; 

   for (RuleCont::const_iterator I = rules_.begin(); I!=rules_.end(); ++I)
   {
      if (!I->matchSource (sourcename))
         continue;

         
      ZLogSink * s = I->getSink (); 

      for (unsigned int i=0; i<zlog::MAX_LEVEL; ++i)
      {
         if (!I->matchLevel (i))
            continue;

         if (s)
            source.setSink (i, s); 

         for (MatchRule::const_iterator F = I->filterBegin(); 
               F!=I->filterEnd(); ++F)
         {
            source.addFilter (*F); 
         }
         
      }

      if (I->isFinal ())
         break; 
   }

   return matched; 
}

void ZLogConfigurator::addRule (const std::string & regex, bool final, 
      const std::string & sink, const std::vector<std::string> & filters)
{
   ZLogSink * s = (sink.size() ? lookupSink (sink) : 0); 

   std::vector<ZLogFilter *> f (filters.size()); 
   ASSERT (f.size() == filters.size()); 
   for (unsigned int i=0; i<f.size(); ++i)
   {
      f[i] = lookupFilter (filters[i]); 
   }

   rules_.push_back (MatchRule (regex, final, s, f)); 
}
   

void ZLogConfigurator::addSink (const char * name, ZLogSink * sink)
{
   ALWAYS_ASSERT(sink); 
   sinks_[std::string(name)] = sink;
   sink->initialize (); 
}

void ZLogConfigurator::addFilter (const char * name, ZLogFilter * filter)
{
   ALWAYS_ASSERT(filter); 
   filters_[std::string(name)] = filter; 
   filter->initialize (); 
}
   
ZLogFilter * ZLogConfigurator::lookupFilter (const std::string & name) const
{
   FilterMap::const_iterator I = filters_.find (name); 
   if (I == filters_.end())
   {
      ZTHROW (ZLogException ()
            << zexception_msg(str(format("Filter not defined: '%s'!") % name)));
   }
   return I->second; 
}

ZLogSink * ZLogConfigurator::lookupSink (const std::string & name) const
{
   SinkMap::const_iterator I = sinks_.find (name); 
   if (I == sinks_.end())
   {
      ZTHROW (ZLogException ()
            << zexception_msg (str(format("Sink not defined: '%s'!") % name)));
   }
   return I->second; 
}

//===========================================================================
   }
}
