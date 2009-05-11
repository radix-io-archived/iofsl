#include <boost/format.hpp>
#include "iofwdutil/IOFWDLog.hh"
#include "iofwdutil/tools.hh"

using namespace iofwdutil;
using namespace boost; 


int main (int UNUSED(argc), char ** UNUSED(args))
{
   for (unsigned int i=0; i<zlog::MAX_LEVEL; ++i)
      IOFWDLog::getSource ().doLog (i, format ("test level %i") % i); 

   return 0; 
}
