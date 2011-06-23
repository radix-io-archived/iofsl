#ifndef IOFWDCLIENT_IOFWDCLIENTCB_HH
#define IOFWDCLIENT_IOFWDCLIENTCB_HH

#include "iofwdevent/CBException.hh"
#include "zoidfs/zoidfs-async.h"

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

