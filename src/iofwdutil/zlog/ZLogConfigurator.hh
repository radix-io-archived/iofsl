#ifndef IOFWDUTIL_ZLOG_ZLOGCONFIGURATOR_HH
#define IOFWDUTIL_ZLOG_ZLOGCONFIGURATOR_HH

#include <string>
#include <vector>
#include <map>

#include <boost/regex.hpp>
#include "LevelMatch.hh"

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

   /// --- rule matching -- 
   void addRule (const std::string & regex, bool final, 
         const std::string & sink, const std::vector<std::string> & filters);
   
   /// -- Clear filters, rules and sinks
   void clear (); 


   /// Apply matched rules. Returns true if any rule matched
   bool configureSource (ZLogSource & source); 
   
   /// Return named filter; 0 if none found
   ZLogFilter * lookupFilter (const std::string & name) const;

   /// Return named sink; 0 if none found
   ZLogSink * lookupSink (const std::string & name) const;

protected:

   class MatchRule
   {
     public:
         typedef std::vector<ZLogFilter *> FC;
         typedef FC::const_iterator const_iterator; 
      public:
         MatchRule (const std::string & regex, bool final,
               ZLogSink * sink, const std::vector<ZLogFilter *> &
               filters);

         bool matchSource (const std::string & name) const; 

         bool matchLevel (int level) const; 

         bool isFinal () const
         { return final_; } 

         ZLogSink * getSink () const
         { return sink_; }


         const_iterator filterBegin () const
         { return filters_.begin(); }

         const_iterator filterEnd () const
         { return filters_.end(); }

      protected:
         LevelMatch levelmatch_; 
         boost::regex sourcematch_; 
         bool final_; 
         FC filters_;
         ZLogSink *  sink_; 
   }; 

private:

   static std::string fixRegEx (const std::string & s); 


protected:
   typedef std::map<std::string,ZLogFilter *> FilterMap; 
   typedef std::map<std::string,ZLogSink *> SinkMap; 

   FilterMap filters_; 
   SinkMap sinks_; 

   typedef std::vector<MatchRule> RuleCont;
   RuleCont rules_; 
}; 

//===========================================================================
   }
}


#endif
