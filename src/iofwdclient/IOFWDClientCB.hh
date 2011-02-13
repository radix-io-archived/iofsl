#ifndef IOFWDCLIENT_IOFWDCLIENTCB_HH
#define IOFWDCLIENT_IOFWDCLIENTCB_HH

#include "src/iofwdevent/CBException.hh"
#include "src/zoidfs/zoidfs-async.h"

#include <boost/function.hpp>

namespace iofwdclient
{
   //========================================================================


   /* called multiple times at diff completion stages of the zoidfs op */
   typedef boost::function<
          void(zoidfs::zoidfs_comp_mask_t, const iofwdevent::CBException & 
               )> IOFWDClientCB;

   //========================================================================
}

#endif

