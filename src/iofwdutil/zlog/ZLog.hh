#ifndef IOFWDUTIL_ZLOG_HH
#define IOFWDUTIL_ZLOG_HH

#include <boost/assert.hpp>
#include <boost/array.hpp>

namespace iofwdutil
{
   namespace zlog
   {
//===========================================================================

// Forward defs
class ZLogSource;
class ZLogSink; 


enum 
{ 
   DEBUG_EXTREME = 0,
   DEBUG_MORE,
   DEBUG,
   INFO,
   WARN,
   CRITICAL,
   ERROR,

   // MAX LEVEL needs to be the number of levels in the structure
   MAX_LEVEL
}; 


class ZLog
{
   public:

      ZLog (); 
      
      static ZLog & get ()
      {
         static ZLog singleton; 
         return singleton; 
      }

      void doConfig (ZLogSource & source); 

      const char * getLevelName (int level) const
      {
         BOOST_ASSERT (level >= 0 && level < (int)_levelNames.size()); 
         return _levelNames[level]; 
      }

   protected:
      ZLogSink * getSinkByName (const char * s); 

   private:
      friend class ZLogSource;

      static boost::array<const char *, MAX_LEVEL> _levelNames; 
};
//===========================================================================


#ifdef ZLOG_ENABLE_ERROR
#define ZLOG_ERROR(cl,txt) cl.doLog<iofwdutil::zlog::ERROR>(txt)
#else
#define ZLOG_ERROR(cl,txt)
#endif

#ifdef ZLOG_ENABLE_CRITICAL
#define ZLOG_CRITICAL(cl,txt) cl.doLog<iofwdutil::zlog::CRITICAL>(txt)
#else
#define ZLOG_CRITICAL
#endif

#define ZLOG_WARN(cl,txt) cl.doLog<iofwdutil::zlog::WARN>(txt)
#define ZLOG_INFO(cl,txt) cl.doLog<iofwdutil::zlog::INFO>(txt)

#ifdef ZLOG_ENABLE_DEBUG
#define ZLOG_DEBUG(cl,txt) cl.doLog<iofwdutil::zlog::DEBUG>(txt)
#else
#define ZLOG_DEBUG
#endif

#ifdef ZLOG_ENABLE_DEBUG_MORE
#define ZLOG_DEBUG_MORE(cl,txt) cl.doLog<iofwdutil::zlog::DEBUG_MORE>(txt)
#else
#define ZLOG_DEBUG_MORE
#endif

#ifdef ZLOG_ENABLE_DEBUG_EXTREME
#define ZLOG_DEBUG_EXTREME(cl,txt) cl.doLog<iofwdutil::zlog::DEBUG_EXTREME>(txt)
#else
#define ZLOG_DEBUG_EXTREME
#endif


//==========================================================================
   }
}

#endif
