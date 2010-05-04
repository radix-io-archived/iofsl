#include <iostream>

#include "zoidfs-ops.hh"
#include "zoidfs-c-util.h"

namespace zoidfs
{

   std::ostream & operator << (std::ostream & out,
         const zoidfs_handle_t & handle)
   {
      char buf[128];
      zoidfs_handle_to_text (&handle, buf, sizeof(buf));
      out << &buf[0];
      return out;
   }

}
