#ifndef IOFWD_FRONTEND_IOFWDFRONTEND_HH
#define IOFWD_FRONTEND_IOFWDFRONTEND_HH

#include <boost/thread.hpp>
#include <boost/smart_ptr.hpp>
#include <boost/array.hpp>

#include <csignal>

#include "Frontend.hh"
#include "iofwdutil/IOFWDLog.hh"
#include "iofwdutil/bmi/BMI.hh"
#include "IOFWDResources.hh"
#include "iofwdevent/Resource.hh"
#include "iofwdevent/CBException.hh"

#include "iofwd/service/Service.hh"

// Service forward declarations
namespace iofwd
{
   class Log;
   class BMI;
   class Config;
}

namespace iofwd
{
   namespace frontend
   {
//===========================================================================

class IOFWDFrontend : public Frontend,
                      public service::Service
{
public:

   IOFWDFrontend (service::ServiceManager & m);

   virtual ~IOFWDFrontend ();

   virtual void init ();

   virtual void run ();

   virtual void destroy ();

protected:

   void handleIncoming (int count, const BMI_unexpected_info * info);

   /// BMIResource callback
   void newUnexpected (iofwdevent::CBException e);

   /// Reregister request with BMI callback
   void post_testunexpected ();

protected:
   boost::shared_ptr<Log> log_service_;
   boost::shared_ptr<BMI> bmi_service_;
   boost::shared_ptr<Config> config_service_;

   iofwdutil::IOFWDLogSource & log_;

   iofwdevent::BMIResource & rbmi_;

private:
   enum { BATCH_SIZE = 128 };

   boost::array<BMI_unexpected_info, BATCH_SIZE> info_;

   // The number of messages completed
   int ue_count_;

   sig_atomic_t stop_;

   // minimum size of valid incoming request
   size_t req_minsize_;

   // The request handle for testunexpected
   iofwdevent::Resource::Handle unexpected_handle_;

protected:
   IOFWDResources res_;
};

//===========================================================================
   }
}


#endif
