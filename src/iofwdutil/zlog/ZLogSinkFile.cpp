#include <iostream>
#include "ZLogSinkFile.hh"
#include "ZLogSource.hh"


namespace iofwdutil
{
   namespace zlog
   {
      ZLogSinkFile::ZLogSinkFile ()
         : zlog_(ZLog::get())
      {
      }

      ZLogSinkFile::~ZLogSinkFile ()
      {
      }


      void ZLogSinkFile::acceptData (int level, const ZLogSource & source,
            const std::string & msg)
      {
         std::cerr << source.getSourceName () << "|" << zlog_.getLevelName (level)
            << "|" << msg << "\n"; 
      }

   }
}
