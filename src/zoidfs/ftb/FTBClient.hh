#ifndef ZOIDFS_FTB_FTBCLIENT_HH
#define ZOIDFS_FTB_FTBCLIENT_HH

#include "iofwdutil/IOFWDLog-fwd.hh"

#include <ftb.h>

namespace zoidfs
{
   //========================================================================

   class ServerSelector;

   //=====================================================================


      class FTBClient
      {
         public:
            FTBClient (ServerSelector & sel);

            ~FTBClient ();

            void poll ();

         protected:
            int checkFTB (int ret) const;

            void parseData (const void * ptr, size_t size);

         protected:
            ServerSelector & sel_;
            iofwdutil::IOFWDLogSource & log_;

            FTB_client_handle_t handle_;
            FTB_subscribe_handle_t sub_;
      };

   //========================================================================
}


#endif
