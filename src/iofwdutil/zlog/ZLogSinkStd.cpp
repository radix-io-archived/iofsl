#include <boost/format.hpp>
#include <iostream>
#include "ZLogException.hh"
#include "ZLogSinkStd.hh"

using namespace boost;

namespace iofwdutil
{
   namespace zlog
   {
//===========================================================================

ZLogSinkStd::ZLogSinkStd ()
{
}

ZLogSinkStd::~ZLogSinkStd ()
{
}

void ZLogSinkStd::setOption (const std::string & name, const std::string &
      val)
{
   // block filename option
   if (name == "filename")
   {
      throw ZLogException (str(format("Invalid option ('%s') for ZLogSinkStd")
               % name));
   }
   ZLogSinkFile::setOption (name, val);
}

void ZLogSinkStd::openFile ()
{
   output_.reset (new std::ostream (stderr_ ? std::cerr.rdbuf() : std::cout.rdbuf()));
   output_->rdbuf()->pubsetbuf(0, 0);
}
//===========================================================================
   }
}

