#ifndef IOFWDUTIL_ZLOG_ZLOGTRACER_HH
#define IOFWDUTIL_ZLOG_ZLOGTRACER_HH

#include <string>

#include "ZLog.hh"
#include "ZLogSource.hh"

namespace iofwdutil
{
   namespace zlog
   {
      //=====================================================================

#define TOKENPASTE(x, y) x ## y
#define TOKENPASTE2(x, y) TOKENPASTE(x, y)

#     define ZLOG_AUTOTRACE(source) \
        iofwdutil::zlog::ZLogAutoTracer \
            TOKENPASTE2(tracer_,__LINE__)  \
            (source,__FILE__,__FUNCTION__,__LINE__);
      
      /**
       * Helper class that automatically traces when a scope is entered/left.
       */
      struct ZLogAutoTracer
      {
         public:
            ZLogAutoTracer (ZLogSource & log, const char * filename, const
                  char * funcname, size_t linenum)
               : log_(log), filename_(filename), funcname_(funcname),
               linenum_(linenum)
            {
               ZLOG_TRACE(log_, getText(true));
            }

            ~ZLogAutoTracer ()
            {
               ZLOG_TRACE(log_, getText(false));
            }

            const std::string getText (bool enter) const;

         protected:
            ZLogSource & log_;
            const char * filename_;
            const char * funcname_;
            size_t       linenum_;
      };
      //=====================================================================
   }
}

#endif
