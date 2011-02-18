#ifndef IOFWDUTIL_IOFWDLOG_HH
#define IOFWDUTIL_IOFWDLOG_HH


// Bring in boost::format because almost everybody uses it
#include <boost/format.hpp>
#include <boost/thread.hpp>
#include <boost/unordered_map.hpp>

#include <memory>
#include "iofwdutil/zlog/ZLogSource.hh"
#include "iofwdutil/zlog/ZLog.hh"
#include "Singleton.hh"
#include "iofwdutil/zlog/ZLogTracer.hh"

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

using boost::format;

typedef zlog::ZLogSource IOFWDLogSource;

class IOFWDLog : public Singleton<IOFWDLog>
{

   friend class Singleton<IOFWDLog>;

public:

   /**
    * @TODO: Return refcounted pointers instead of reference, so we can
    * properly cleanup/release memory.
    *
    * Note: for now, don't try to delete the returned reference!
    * (For that matter, never try to delete &some_reference !)
    */
    static zlog::ZLogSource & getSource (const char * sourcename = 0)
    {
       if (!sourcename)
          return IOFWDLog::instance().getDefaultSource ();

       return IOFWDLog::instance().getSourceInt (sourcename);
    }


    // Has to be public for scoped_ptr
    ~IOFWDLog ();

private:

    IOFWDLog ();

    zlog::ZLogSource & getDefaultSource ()
    {
       return *default_;
    }

    zlog::ZLogSource & getSourceInt (const char * name)
    {
       boost::mutex::scoped_lock l(lock_);

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
    typedef boost::unordered_map<std::string,
                        zlog::ZLogSource *> SourceMapType;
    SourceMapType sources_;

    std::auto_ptr<zlog::ZLogConfigurator> configurator_;

    unsigned int default_loglevel_;

    std::vector<std::pair<std::string, unsigned int> > loglevel_override_;

    zlog::ZLogSource * default_;

    boost::mutex lock_;
};


/**
 * Helper for easy tracing
 */

/// Default log name
#define ZLOG_DEFAULT iofwdutil::IOFWDLog::getSource()

/// Autotrace to default log
#define ZLOG_AUTOTRACE_DEFAULT ZLOG_AUTOTRACE(ZLOG_DEFAULT)

//===========================================================================
}

#endif
