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

#define ZLOG_COMMON(cl,level,txt) \
   if (cl.isEnabled(level)) \
   { cl.doLog(level,txt); }

#ifndef ZLOG_DISABLE_ERROR
#define ZLOG_ERROR(cl,txt) ZLOG_COMMON(cl,iofwdutil::zlog::ERROR,txt)
#else
#define ZLOG_ERROR(cl,txt)
#endif

#ifndef ZLOG_DISABLE_CRITICAL
#define ZLOG_CRITICAL(cl,txt) ZLOG_COMMON(cl,iofwdutil::zlog::CRITICAL,txt)
#else
#define ZLOG_CRITICAL(cl,txt)
#endif

#ifndef ZLOG_DISABLE_WARN
#define ZLOG_WARN(cl,txt) ZLOG_COMMON(cl,iofwdutil::zlog::WARN,txt)
#else
#define ZLOG_WARN(cl,txt) 
#endif

#ifndef ZLOG_DISABLE_INFO
#define ZLOG_INFO(cl,txt) ZLOG_COMMON(cl,iofwdutil::zlog::INFO,txt)
#else
#define ZLOG_INFO(cl,txt)
#endif


#ifndef ZLOG_DISABLE_DEBUG
#define ZLOG_DEBUG(cl,txt) ZLOG_COMMON(cl,iofwdutil::zlog::DEBUG,txt)
#else
#define ZLOG_DEBUG(cl,txt)
#endif

#ifndef ZLOG_DISABLE_DEBUG_MORE
#define ZLOG_DEBUG_MORE(cl,txt) ZLOG_COMMON(cl,iofwdutil::zlog::DEBUG_MORE,txt)
#else
#define ZLOG_DEBUG_MORE(cl,txt)
#endif

#ifndef ZLOG_DISABLE_DEBUG_EXTREME
#define ZLOG_DEBUG_EXTREME(cl,txt) ZLOG_COMMON(cl,iofwdutil::zlog::DEBUG_EXTREME,txt)
#else
#define ZLOG_DEBUG_EXTREME(cl,txt)
#endif


/* 
 * check if all logging is disabled... if so... redef all the logging commands 
 * to empty commands 
 */
#ifdef ZLOG_DISABLE_ALL

#undef ZLOG_ERROR
#define ZLOG_ERROR(cl,txt)

#undef ZLOG_CRITICAL
#define ZLOG_CRITICAL(cl,txt)

#undef ZLOG_WARN
#define ZLOG_WARN(cl,txt)

#undef ZLOG_INFO
#define ZLOG_INFO(cl,txt)

#undef ZLOG_DEBUG
#define ZLOG_DEBUG(cl,txt)

#undef ZLOG_DEBUG_MORE
#define ZLOG_DEBUG_MORE(cl,txt)

#undef ZLOG_DEBUG_EXTREME
#define ZLOG_DEBUG_EXTREME(cl,txt)

#endif /* ZLOG_DISABLE_ALL */

//==========================================================================
   }
}

#endif
