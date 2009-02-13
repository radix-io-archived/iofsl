#include "Request.hh"
#include "common/zoidfs-wrapped.hh"


namespace iofwd
{
//===========================================================================

Request::Request (int opid)
   : opid_(opid), status_(ZFS_OK)
{
}

Request::~Request ()
{
   // maybe log here 
}

//===========================================================================
}
