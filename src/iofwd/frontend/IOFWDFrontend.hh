#ifndef IOFWD_FRONTEND_IOFWDFRONTEND_HH
#define IOFWD_FRONTEND_IOFWDFRONTEND_HH

#include <boost/thread.hpp>
#include <boost/smart_ptr.hpp>
#include <boost/array.hpp>

#include "Frontend.hh"
#include "iofwdutil/IOFWDLog.hh"
#include "iofwdutil/completion/BMIResource.hh"
#include "iofwd/Resources.hh"
#include "iofwdutil/bmi/BMI.hh"
#include "iofwdutil/bmi/BMIContext.hh"
#include "IOFWDResources.hh"

namespace iofwd
{
   namespace frontend
   {
//===========================================================================

class IOFWDFrontend : public Frontend
{
public:

   /// res = the BMI resource to use
   IOFWDFrontend (iofwdutil::completion::BMIResource & res,
         Resources & r);

   virtual ~IOFWDFrontend ();

   virtual void init ();

   virtual void run ();

   virtual void destroy ();

protected:

   void handleIncoming (int count, const BMI_unexpected_info * info);

   /// BMIResource callback
   void newUnexpected (int status);

   /// Reregister request with BMI callback
   void post_testunexpected ();

protected:
   iofwdutil::IOFWDLogSource & log_;

   iofwdutil::completion::BMIResource & bmires_;

   Resources & r_;

   iofwdutil::bmi::BMIContextPtr bmictx_;


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
