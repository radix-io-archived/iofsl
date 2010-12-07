#include <openssl/evp.h>
#include "OpenSSLInit.hh"

namespace iofwdutil
{
   namespace hash
   {


OpenSSLInit::OpenSSLInit ()
{
   OpenSSL_add_all_digests();
}

   }
}
