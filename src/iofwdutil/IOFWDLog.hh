#ifndef IOFWDUTIL_IOFWDLOG_HH
#define IOWFDUTIL_IOFWDLOG_HH

#include "iofwdutil/zlog/ZLog.hh"
#include "Singleton.hh"

namespace iofwdutil
{
//===========================================================================

class IOFWDLog : public Singleton<IOFWDLog>
{ 

   friend class Singleton<IOFWDLog>;

public:

    static zlog::ZLog & get ()
    {
       return IOFWDLog::instance().log_; 
    }

    static zlog::ZLogSource & getSource (const char * sourcename)
    {
       return IOFWDLog::instance().getSource (sourcename); 
    }
    
    
    // Has to be public for scoped_ptr
    ~IOFWDLog (); 

private:
  
    IOFWDLog (); 


    zlog::ZLogSource & getSourceInt (const char * name);

    void setDefaultConfig (zlog::ZLogSource & source); 

    //void loadRules (); 


private:
    zlog::ZLog log_; 
};

//===========================================================================
}

#endif
