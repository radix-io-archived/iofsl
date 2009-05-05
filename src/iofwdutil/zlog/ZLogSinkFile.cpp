#include <iostream>
#include <boost/format.hpp>
#include "ZLogSinkFile.hh"
#include "ZLogSource.hh"
#include "ZLogException.hh"

namespace iofwdutil
{
   namespace zlog
   {
//===========================================================================

ZLogSinkFile::ZLogSinkFile (ZLog & log)
   : zlog_(log), output_(0)
{
}

ZLogSinkFile::~ZLogSinkFile ()
{
}

void ZLogSinkFile::initialize ()
{
   if (filename_.empty())
      throw ZLogException ("ZLogSinkFile needs a configured filename! "
            "(option filename)\n"); 
   output_.open (filename_.c_str()); 
   if (!output_)
   {
      throw ZLogException (str(boost::format("ZLogSinkFile: error opening"
                  " file '%s'\n") % filename_)); 
                  
   }
}

void ZLogSinkFile::setOption (const std::string & name, const
      std::string & val)
{
   if (name == "filename")
      filename_ = val; 
   else
      throw ZLogException (str(boost::format ("Invalid option ('%s') for ZLogSinkFile!") % name));
}


void ZLogSinkFile::acceptData (int level, const ZLogSource & source,
      const std::string & msg)
{
   output_ << source.getSourceName () << "|" << zlog_.getLevelName (level)
      << "|" << msg << "\n"; 
}

//===========================================================================
   }
}
