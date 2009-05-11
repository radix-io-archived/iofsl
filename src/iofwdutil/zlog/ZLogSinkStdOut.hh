#ifndef IOFWDUTIL_ZLOG_ZLOGSINKSTDOUT_HH
#define IOFWDUTIL_ZLOG_ZLOGSINKSTDOUT_HH

#include "ZLogSinkStd.hh"

namespace iofwdutil
{
   namespace zlog
   {

      class ZLogSinkStdOut : public ZLogSinkStd 
      {
      public:
         ZLogSinkStdOut ()
         {
            stderr_ = false; 
         }

         virtual ~ZLogSinkStdOut (); 
      };

   }
}
#endif
