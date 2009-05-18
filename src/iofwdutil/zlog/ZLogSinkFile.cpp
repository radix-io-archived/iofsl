#include <iostream>
#include <boost/format.hpp>
#include "iofwdutil/tools.hh"
#include "ZLogSinkFile.hh"
#include "ZLogSource.hh"
#include "ZLogException.hh"

namespace iofwdutil
{
   namespace zlog
   {
//===========================================================================

ZLogSinkFile::ZLogSinkFile ()
   : output_(0)
{
}

ZLogSinkFile::~ZLogSinkFile ()
{
}

void ZLogSinkFile::openFile ()
{
   if (filename_.empty())
      throw ZLogException ("ZLogSinkFile needs a configured filename! "
            "(option filename)\n"); 
   output_.reset (new std::ofstream (filename_.c_str())); 
}

void ZLogSinkFile::initialize ()
{
   openFile (); 
   if (! *output_)
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


void ZLogSinkFile::acceptData (int UNUSED(level), const ZLogSource & UNUSED(source),
      const std::string & msg)
{
   boost::mutex::scoped_lock l (outputlock_); 
   *output_ << msg;
   output_->flush (); 
}

//===========================================================================
   }
}
