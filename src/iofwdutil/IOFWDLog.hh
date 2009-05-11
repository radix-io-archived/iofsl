#ifndef IOFWDUTIL_IOFWDLOG_HH
#define IOWFDUTIL_IOFWDLOG_HH

#include <memory>
#include "iofwdutil/zlog/ZLogSource.hh"
#include "iofwdutil/zlog/ZLog.hh"
#include "Singleton.hh"

namespace iofwdutil
{
   namespace zlog
   {

      class ZLogConfigurator;
      class ZLogFilter;
      class ZLogSink;

   }
}

namespace iofwdutil
{
//===========================================================================

class IOFWDLog : public Singleton<IOFWDLog>
{ 

   friend class Singleton<IOFWDLog>;

public:

    static zlog::ZLogSource & getSource (const char * sourcename = "default")
    {
       return IOFWDLog::instance().getSourceInt (sourcename); 
    }
    
    
    // Has to be public for scoped_ptr
    ~IOFWDLog (); 

private:
  
    IOFWDLog (); 


    zlog::ZLogSource & getSourceInt (const char * name)
    {
        SourceMapType::const_iterator I = sources_.find (name); 
        if (I!=sources_.end())
           return *I->second;
        return *createSource (name); 
    }


    zlog::ZLogSource * createSource (const char * name);

    void setDefaultConfig (zlog::ZLogSource & source); 

    //void loadRules (); 

    void setLogLevelOverride (const std::string & str); 

    void createDefaultSinks (); 

    void createDefaultFilters (); 

    void applyLogLevelOverride (zlog::ZLogSource & source); 

private:
    typedef std::map<std::string, zlog::ZLogSource *> SourceMapType; 
    SourceMapType sources_; 

    std::auto_ptr<zlog::ZLogConfigurator> configurator_; 

    unsigned int default_loglevel_; 

    std::vector<std::pair<std::string, unsigned int> > loglevel_override_; 

};

//===========================================================================
}

#endif
