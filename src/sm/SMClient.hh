#ifndef SM_SMCLIENT_HH
#define SM_SMCLIENT_HH

#include <boost/intrusive_ptr.hpp>
#include "iofwdutil/IntrusiveHelper.hh"
#include "iofwdevent/CBException.hh"
namespace sm
{
//===========================================================================

class SMClient : public iofwdutil::IntrusiveHelper
{
public:

   /**
    * Called when the client is scheduled. It can run as long as
    * desired; If false is returned, it will immediately be rescheduled.
    */
   virtual bool execute () = 0;
   virtual ~SMClient ();
};

INTRUSIVE_PTR_HELPER(SMClient);

typedef boost::intrusive_ptr<SMClient> SMClientSharedPtr;

//===========================================================================
}

#endif
