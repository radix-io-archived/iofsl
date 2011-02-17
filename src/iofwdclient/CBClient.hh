#ifndef IOFWDCLIENT_CBCLIENT_HH
#define IOFWDCLIENT_CBCLIENT_HH

#include "iofwdclient/IOFWDClientCB.hh"
#include "iofwdclient/clientsm/GetAttrClientSM.hh"

#include "zoidfs/zoidfs-async.h"

#include "iofwdutil/IOFWDLog-fwd.hh"

#include "sm/SMManager.hh"

#include <boost/scoped_ptr.hpp>

#include <queue>


namespace iofwdclient
{
   //========================================================================

   /**
    * Implements a callback version of the async zoidfs API
    */
   class CBClient
   {
      public:
         CBClient (bool poll = true);

         ~CBClient ();

      public:

         void cbgetattr (const IOFWDClientCB & cb,
               int * ret,
               const zoidfs::zoidfs_handle_t * handle,
               zoidfs::zoidfs_attr_t * attr,
               zoidfs::zoidfs_op_hint_t * op_hint);


      protected:
         iofwdutil::IOFWDLogSource & log_;
         boost::scoped_ptr<sm::SMManager> smm_;
         bool poll_;

         std::queue<iofwdclient::clientsm::GetAttrClientSM *> temp_smclient_queue_;
   };

   //========================================================================
}

#endif
