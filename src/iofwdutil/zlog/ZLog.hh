#ifndef IOFWDUTIL_ZLOG_HH
#define IOFWDUTIL_ZLOG_HH

#include <string>
#include <boost/assert.hpp>
#include <boost/array.hpp>
#include <map>

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
   ERROR = 0,   // cannot be disabled
   CRITICAL,
   WARN,
   INFO,
   DEBUG,
   DEBUG_MORE,
   DEBUG_EXTREME,

   // MAX LEVEL needs to be the number of levels in the structure
   MAX_LEVEL
}; 


class ZLog
{
   public:

      ZLog (); 
      
      ~ZLog (); 


      static const char * getLevelName (int level) 
      {
         BOOST_ASSERT (level >= 0 && level < (int)levelNames_.size()); 
         return levelNames_[level]; 
      }


      /// Convert level string to level value. return -1 if invalid level name
      static int name2Level (const std::string & levelname); 


      /// Return true if simplified regex matches sourcename
      static bool matchSourceName (const std::string & regex, const std::string
            & sourcename); 

      /// Convert simplified regex into regex
      static std::string filterRegEx (const std::string & reg);


   private:
      friend class ZLogSource;

      static boost::array<const char *, MAX_LEVEL> levelNames_;
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
