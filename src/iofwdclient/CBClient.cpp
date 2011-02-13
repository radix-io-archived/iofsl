#include "iofwdclient/CBClient.hh"

#include "iofwdutil/IOFWDLog.hh"
#include "iofwdutil/tools.hh"

using namespace zoidfs;

namespace iofwdclient
{
   //========================================================================

   CBClient::CBClient ()
      : log_ (iofwdutil::IOFWDLog::getSource ())
   {
   }

   CBClient::~CBClient ()
   {
   }

   void CBClient::cbgetattr (const IOFWDClientCB & UNUSED(cb),
         int * UNUSED(ret),
         const zoidfs_handle_t * UNUSED(handle),
         zoidfs_attr_t * UNUSED(attr),
         zoidfs_op_hint_t * UNUSED(op_hint))
   {
   }

   //========================================================================
}
