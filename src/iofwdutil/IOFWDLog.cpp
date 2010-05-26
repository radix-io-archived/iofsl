#include "iofwdutil/boost-spirit.hh"

#include <boost/format.hpp>
#include <boost/lambda/lambda.hpp>

#include <algorithm>
#include <iostream>

#include "IOFWDLog.hh"
#include "tools.hh"
#include "iofwdutil/zlog/ZLogConfigurator.hh"
#include "iofwdutil/zlog/ZLogSinkStdOut.hh"
#include "iofwdutil/zlog/ZLogSinkStdErr.hh"
#include "iofwdutil/zlog/ZLogSource.hh"
#include "iofwdutil/zlog/ZLogDefaultFilter.hh"
#include "iofwdutil/zlog/LevelParser.hh"

using namespace iofwdutil::zlog;
using namespace ourspirit;
using namespace ourphoenix;
using namespace boost;

namespace iofwdutil
{
//===========================================================================

zlog::ZLogSource * IOFWDLog::createSource (const char * name)
{
   ZLogSource * newsource = new ZLogSource (name); 

   ALWAYS_ASSERT(sources_.count (name) == 0); 

   sources_[name] = newsource; 

   if (!configurator_->configureSource (*newsource))
        setDefaultConfig (*newsource); 

   applyLogLevelOverride (*newsource); 

   return newsource; 
}


IOFWDLog::IOFWDLog ()
   : configurator_ (new ZLogConfigurator ()),
     default_loglevel_(INFO)
{
   createDefaultSinks (); 
   createDefaultFilters (); 

   // Set log levels from env
   if (getenv ("IOFWD_LOGLEVEL"))
      setLogLevelOverride (getenv("IOFWD_LOGLEVEL")); 
}

void IOFWDLog::createDefaultFilters ()
{
   configurator_->addFilter ("default", new ZLogDefaultFilter ()); 
}

/*
 * Loglevel string is of the following form:
 *  
 *    [ [sourcematch:] loglevel ] [, [sourcematch:] loglevel] ...
 *
 *    if sourcematch is missing, '*' is assumed
 */

namespace 
{

class LevelStrParser : public grammar<LevelStrParser>
{
public:
   typedef std::pair<std::string, unsigned int> LogLevelEntry; 
   typedef std::vector<LogLevelEntry> VectorType; 

   LevelStrParser (VectorType & entries) : 
      entries_ (entries)
   {
   }

   struct myclosure : ourspirit::closure<myclosure, unsigned int,
   std::string> 
   {
      member1 level;
      member2 name; 
   }; 

   void addEntry (unsigned int level, const std::string & name) const
   {
      //std::cerr << str(format("Adding entry '%s' '%u'\n") % name % level); 
      entries_.push_back (std::make_pair(name, level)); 
   }

   static void setName2 (std::string & name, const char * const & p1, const char * const & p2)
   {
      name = std::string(p1,p2); 
   }
   
   static void setName (std::string & name, const std::string & p1)
   {
      name = std::string(p1); 
   }
   
   static void clearName (std::string & name, const char * const & , const char * const & )
   {
      name.clear (); 
   }

   template <typename ScannerT>
   struct definition 
   {
      definition (LevelStrParser const & self) : level_p()
      {
         // if level_p is not excluded, when parsing "error" (or any other
         // levelname) entry.name_ is first set before the rule fails because
         // of ':'. When calling addEntry the name is still there.
         // Therefore, added a epsilon transition which is called when no
         // valid string + ':' is found. The epsilon action clears the name.
         entry_ = ( 
                    (( (+chset_p("A-Z_\\-a-z0-9.*") )
                       [
                       bind(&LevelStrParser::setName2)(entry_.name,
                          arg1, arg2)
                       ] >> ':' ) 
                      | eps_p[bind(&LevelStrParser::clearName)(entry_.name,arg1,arg2)]
                     )
                      >> level_p[entry_.level = ourphoenix::arg1]
                 )[bind(&LevelStrParser::addEntry)(self, entry_.level, entry_.name)]
               ; 

         start_ = !entry_ >> *( ',' >>  entry_); 
      }

      rule<ScannerT> const & start () const
      { return start_; }

      rule<ScannerT,myclosure::context_t> entry_; 
      rule<ScannerT> start_; 

      const LevelParser level_p; 
   }; 

   VectorType & entries_; 
}; 

}

void IOFWDLog::setLogLevelOverride (const std::string & source)
{
   LevelStrParser parser (loglevel_override_); 

   if (!parse (source.c_str(), parser, space_p).full)
   {
      ALWAYS_ASSERT(false && "Invalid loglevel string"); 
   }

   if (loglevel_override_.empty())
      return; 

   /*
    * remove global entries from list
    * (Global entries are 'debug' (and not 'localcache:debug')
    *
    * Walk backwards over the vector so we can remove while going through the
    * array. The first global entry in the loglevel string will set the
    * default loglevel
    * */
   for (int i=static_cast<int>(loglevel_override_.size())-1; i>=0; --i)
   {
      if (loglevel_override_[i].first.size()== 0)
      {
         // set default loglevel
         default_loglevel_ = loglevel_override_[i].second; 
         loglevel_override_.erase (loglevel_override_.begin()+i); 
      }
   }
}

void IOFWDLog::applyLogLevelOverride (zlog::ZLogSource & source)
{
   for (unsigned int i=0; i<loglevel_override_.size(); ++i)
   {
      const unsigned int level = loglevel_override_[i].second; 
      const std::string & regex = loglevel_override_[i].first; 

      if (ZLog::matchSourceName (regex, source.getSourceName()))
         source.setLogLevel (level); 
   }
}

void IOFWDLog::createDefaultSinks ()
{
   configurator_->addSink ("stdout", new ZLogSinkStdOut ()); 
   configurator_->addSink ("stderr", new ZLogSinkStdErr ()); 

}

IOFWDLog::~IOFWDLog ()
{
   for (SourceMapType::const_iterator I = sources_.begin(); 
         I!=sources_.end(); ++I)
   {
      delete (I->second); 
   }
}

void IOFWDLog::setDefaultConfig (zlog::ZLogSource & source)
{
   // By default, all output goes to stderr
   for (unsigned int i=0; i<MAX_LEVEL; ++i)
      source.setSink (i, configurator_->lookupSink ("stderr")); 

   source.addFilter (configurator_->lookupFilter ("default")); 

   source.setLogLevel (default_loglevel_); 
}
//===========================================================================
}
