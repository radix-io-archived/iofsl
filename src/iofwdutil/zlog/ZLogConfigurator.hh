#ifndef IOFWDUTIL_ZLOG_ZLOGCONFIGURATOR_HH
#define IOFWDUTIL_ZLOG_ZLOGCONFIGURATOR_HH

#include <string>
#include <vector>
#include <map>

namespace iofwdutil
{
   namespace zlog
   {
//===========================================================================

// Forward
class ZLogSource; 
class ZLogSink;
class ZLogFilter;

/**
 * Implements rule matching for redirecting sources and adding filters
 */
class ZLogConfigurator
{
public:
   ZLogConfigurator (); 

   ~ZLogConfigurator (); 

public:
   // -- Sinks and Filters --
   
   /// Add named sink: ownership not transferred
   void addSink (const char * name, ZLogSink * sink);

   /// Add named filter: ownership not transferred
   void addFilter (const char * name, ZLogFilter * filter); 

public:
   // --- rule matching -- 
   void addRule (const std::string & regex, bool final, 
         const std::string & sink, const std::vector<std::string> & filters);
   
   // -- Clear filters, rules and sinks
   void clear (); 

public:

   /// Apply matched rules. Returns true if any rule matched
   bool configureSource (ZLogSource & source); 

protected:

   class MatchRule
   {
      public:
//         MatchRule (const std::string & sourcematch,
 //              const LevelMatch & levelmatch
      
   }; 

protected:
   std::map<std::string,ZLogFilter *> filters_; 
   std::map<std::string,ZLogSink *> sinks_; 
   std::vector<MatchRule> rules_; 
}; 

//===========================================================================
   }
}

#endif
