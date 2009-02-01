#ifndef IOFWD_FRONTEND_HH
#define IOFWD_FRONTEND_HH

#include "iofwdutil/zoidfs-int.hh"

namespace iofwd
{
   namespace frontend
   {
//===========================================================================

class Frontend : public iofwdutil::ZoidFS
{
   public:

      virtual ~Frontend ();
};

//===========================================================================
   }
}

#endif
