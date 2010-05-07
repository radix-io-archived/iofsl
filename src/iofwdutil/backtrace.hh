#ifndef IOFWDUTIL_BACKTRACE_HH
#define IOFWDUTIL_BACKTRACE_HH

#include <vector>
#include <string>

namespace iofwdutil
{

   /**
    * Fill the vector with the stack trace.
    * Returns an empty vector if no stack traces can be generated on this
    * system.
    */
   void getStackTrace (std::vector<std::string> & vec, 
         size_t maxdepth = 24);


}

#endif
