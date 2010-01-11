#ifndef SM_SMRESOURCEOP_HH
#define SM_SMRESOURCEOP_HH

#include <boost/bind.hpp>
#include <csignal>

#include "SMResourceClient.hh"
#include "iofwdevent/Resource.hh"


namespace sm
{
//===========================================================================

class SMResourceClient;
class SMManager;

/**
 * This resourceop implementation reschedules an entity
 * when the resourceop completes.
 *
 * The entity needs to be an instance of SMResourceClient, having a
 * method receiving the result of the nonblocking operation.
 */
class SMResourceOp
{
public:
   SMResourceOp (SMManager * manager);


   void callback (int status)
   {
      switch (status)
      {
         case iofwdevent::COMPLETED:
            success ();
            break;
         case iofwdevent::CANCELLED:
            cancel ();
            break;
         default:
            ALWAYS_ASSERT(false);
      }
   }

   iofwdevent::CBType callbackRef ()
   { return boost::bind (&SMResourceOp::callback, boost::ref(*this), _1); }

   virtual void success ();

   virtual void cancel ();

   virtual ~SMResourceOp ();


   void rearm (SMResourceClientSharedPtr client);

protected:
   void reschedule (bool success);

protected:
   SMManager * manager_;

   // By using a shared pointer, we insure nobody is going to delete
   // the client while there are still operations pending.
   SMResourceClientSharedPtr client_;

   sig_atomic_t completed_;
};

//===========================================================================
}
#endif
