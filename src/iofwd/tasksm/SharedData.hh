#ifndef IOFWD_TASKSM_SHAREDDATA_HH
#define IOFWD_TASKSM_SHAREDDATA_HH

#include "sm/sm-fwd.hh"

// @TODO: move into proper forward include header
namespace zoidfs
{
    namespace util
    {
        class ZoidFSAsync;
    }
}

namespace iofwd
{
   namespace tasksm
   {
      //=====================================================================
      /**
       * Convenience structure for passing data to the tasksm state machines
       */
      struct SharedData
      {

         SharedData (zoidfs::util::ZoidFSAsync * a,
               sm::SMManager & sm)
            : api(a), smm(sm)
         {
         }


         zoidfs::util::ZoidFSAsync * api;
         sm::SMManager & smm;

      };

      //=====================================================================
   }
}

#endif
