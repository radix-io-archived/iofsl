#include <iostream>
#include <cstdlib>

#include "always_assert.hh"



void iofwdutil::always_assert (bool t, unsigned int line, const char * file,
      const char * msg)
{
   if (t)
      return;

   std::cerr << "ALWAYS_ASSERT (" << msg << ") failed! (line " << line 
      << ", file " << file << ")!\n"; 
   std::exit (1); 
}
