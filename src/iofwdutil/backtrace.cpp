#include <cstdlib>
#include "iofwd_config.h"

#ifdef HAVE_EXECINFO_H
#include <execinfo.h>
#endif

#include "backtrace.hh"

using namespace std;

namespace iofwdutil
{
   //========================================================================


   void getStackTrace (std::vector<std::string> & vec, size_t maxdepth)
   {
#ifdef HAVE_EXECINFO_H
      std::vector<void *> addrs (maxdepth);
      size_t count = backtrace (&addrs[0], addrs.size());
      char ** symbols = backtrace_symbols(&addrs[0], count);
      vec.reserve (count);
      vec.clear();
      for (size_t i=0; i<count; ++i)
      {
         vec.push_back (symbols[i]);
      }

      free(symbols);
#else
      vec.clear ();
#endif
   }

   //========================================================================
}
