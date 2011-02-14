#include <cstddef> // size_t
#include "Group.hh"

namespace net
{
   //========================================================================

   size_t Group::size () const
   {
      return addrs_.size ();
   }

   //========================================================================
}


