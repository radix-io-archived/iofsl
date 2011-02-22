#include "BMIRPCHelper.hh"
#include "rpc/bmi/BMIConnector.hh"
#include "BMI.hh"
#include "iofwdutil/IOFWDLog.hh"

SERVICE_REGISTER(iofwd::BMIRPCHelper, bmirpchelper);

namespace iofwd
{
   //========================================================================

   BMIRPCHelper::BMIRPCHelper (service::ServiceManager & m)
      : service::Service (m),
        bmi_service_ (lookupService<BMI> ("bmi")),
        connector_ (new rpc::bmi::BMIConnector (bmi_service_->get(),
                 iofwdutil::IOFWDLog::getSource ("rpc")))
   {
      // The connector uses BMI, so we depend on BMIResource to ensure it
      // started.
   }

   BMIRPCHelper::~BMIRPCHelper ()
   {
   }

   //========================================================================
}
