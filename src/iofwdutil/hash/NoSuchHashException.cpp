#include <boost/format.hpp>
#include "NoSuchHashException.hh"

using boost::format;

namespace iofwdutil
{
   namespace hash
   {

      NoSuchHashException::NoSuchHashException (const std::string & hashfunc)
         : ZException (str(format("No hash function '%s' registered!") %
                  hashfunc))
      {
      }

   }
}
