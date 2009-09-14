#include "CFKeyMissingException.hh"

namespace iofwdutil
{


CFKeyMissingException::CFKeyMissingException (const std::string & n)
   : keyname_(n)
{
}

CFKeyMissingException::~CFKeyMissingException ()
{
}

}
