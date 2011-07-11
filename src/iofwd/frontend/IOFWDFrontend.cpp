#include "IOFWDFrontend.hh"

#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <boost/array.hpp>
#include <iostream>

#include "iofwdevent/BMIResource.hh"

#include "iofwdutil/assert.hh"
#include "iofwdutil/bmi/BMIUnexpectedBuffer.hh"
#include "iofwdutil/IOFWDLog.hh"

#include "zoidfs/util/zoidfs-wrapped.hh"
#include "zoidfs/zoidfs-proto.h"

#include "encoder/xdr/XDRReader.hh"
#include "encoder/xdr/XDRSizeProcessor.hh"

// Services
#include "iofwd/Log.hh"
#include "iofwd/Config.hh"
#include "iofwd/BMI.hh"
#include "iofwd/ConfigException.hh"
#include "zoidfs/zoidfs-comm.h"
#include "IOFWDNotImplementedRequest.hh"
#include "IOFWDNullRequest.hh"
#include "IOFWDGetAttrRequest.hh"
#include "IOFWDSetAttrRequest.hh"
#include "IOFWDLookupRequest.hh"
#include "IOFWDReadLinkRequest.hh"
#include "IOFWDCommitRequest.hh"
#include "IOFWDCreateRequest.hh"
#include "IOFWDRemoveRequest.hh"
#include "IOFWDRenameRequest.hh"
#include "IOFWDSymLinkRequest.hh"
#include "IOFWDMkdirRequest.hh"
#include "IOFWDReadDirRequest.hh"
#include "IOFWDResizeRequest.hh"
#include "IOFWDWriteRequest.hh"
#include "IOFWDReadRequest.hh"
#include "IOFWDLinkRequest.hh"

#include "iofwdutil/mm/BMIMemoryManager.hh"
#include "iofwdutil/mm/NBIOMemoryManager.hh"

using namespace iofwdutil::bmi;
using namespace iofwdutil;
using namespace zoidfs;

SERVICE_REGISTER(iofwd::frontend::IOFWDFrontend, bmifrontend);

namespace iofwd
{
   namespace frontend
   {

//===========================================================================

// Anonymous namespace
namespace
{
//===========================================================================
// ==========================================================================
// map op id to IOFWDRequest

typedef Request * (*mapfunc_t) (
      int i, const BMI_unexpected_info & info,
      IOFWDResources & res);


template <typename T>
static inline Request * newreq (int i, const BMI_unexpected_info & info,
      IOFWDResources & res)
{ return new T (i, info, res); }

static boost::array<mapfunc_t, ZOIDFS_PROTO_MAX> map_ = {
   {
      &newreq<IOFWDNullRequest>,
      &newreq<IOFWDGetAttrRequest>,
      &newreq<IOFWDSetAttrRequest>,
      &newreq<IOFWDLookupRequest>,
      &newreq<IOFWDReadLinkRequest>,
      &newreq<IOFWDCommitRequest>,
      &newreq<IOFWDCreateRequest>,
      &newreq<IOFWDRemoveRequest>,
      &newreq<IOFWDRenameRequest>,
      &newreq<IOFWDSymLinkRequest>,
      &newreq<IOFWDMkdirRequest>,
      &newreq<IOFWDReadDirRequest>,
      &newreq<IOFWDResizeRequest>,
      &newreq<IOFWDWriteRequest>,
      &newreq<IOFWDReadRequest>,
      &newreq<IOFWDLinkRequest>
   }
};

// ==========================================================================
//===========================================================================
}

//===========================================================================
//===========================================================================
//===========================================================================

IOFWDFrontend::IOFWDFrontend (service::ServiceManager & m)
   : service::Service (m),
     log_service_ (lookupService<Log>("log")),
     bmi_service_ (lookupService<BMI>("bmi")),
     config_service_ (lookupService<Config>("config")),
     log_(log_service_->getSource ("iofwdfrontend")),
     rbmi_ (bmi_service_->get ()),
   stop_(false),
   req_minsize_(encoder::xdr::getXDRSize (uint32_t ()).getActualSize()),
   res_ (rbmi_, log_)
{
}

IOFWDFrontend::~IOFWDFrontend ()
{
   ZLOG_INFO (log_, "Stopping BMI memory manager...");
   iofwdutil::mm::BMIMemoryManager::instance().reset();
   delete &iofwdutil::mm::BMIMemoryManager::instance();

   ZLOG_INFO (log_, "Stopping NBIO memory manager...");
   iofwdutil::mm::NBIOMemoryManager::instance().reset();
   delete &iofwdutil::mm::NBIOMemoryManager::instance();

}

void IOFWDFrontend::init ()
{
   stop_ = false;

   // @TODO Move this into BMI service

   /* start the BMI memory manager */
   ZLOG_INFO (log_, "Starting BMI memory manager...");
   iofwdutil::ConfigFile lc = config_.openSectionDefault("bmimemorymanager");
   iofwdutil::mm::BMIMemoryManager::instance().setMaxNumBuffers(lc.getKeyAsDefault("maxnumbuffers", 0));
   iofwdutil::mm::BMIMemoryManager::instance().setWarnNumBuffers(lc.getKeyAsDefault("warnnumbuffers", 0));
   /* set the max amount of BMI memory allocs by total bytes (def is 256 MB) */
   iofwdutil::mm::BMIMemoryManager::instance().setMaxMemAmount(lc.getKeyAsDefault("maxmem", 256UL * 1024UL * 1024UL));
   iofwdutil::mm::BMIMemoryManager::instance().setMemWarnAmount(lc.getKeyAsDefault("warnmem", 4UL * 1024UL * 1024UL));
   iofwdutil::mm::BMIMemoryManager::instance().start();

   /* start the NBIO memory manager */
   ZLOG_INFO (log_, "Starting NBIO memory manager...");
   lc = config_.openSectionDefault("nbiomemorymanager");
   iofwdutil::mm::NBIOMemoryManager::instance().setMaxNumBuffers(lc.getKeyAsDefault("maxnumbuffers", 0));
   iofwdutil::mm::NBIOMemoryManager::instance().setWarnNumBuffers(lc.getKeyAsDefault("warnnumbuffers", 0));
   /* set the max amount of NBIO memory allocs by total bytes (def is 256 MB) */
   iofwdutil::mm::NBIOMemoryManager::instance().setMaxMemAmount(lc.getKeyAsDefault("maxmem", 256UL * 1024UL * 1024UL));
   iofwdutil::mm::NBIOMemoryManager::instance().setMemWarnAmount(lc.getKeyAsDefault("warnmem", 4UL * 1024UL * 1024UL));
   bool zc = lc.getKeyAsDefault<bool>("zerocopy", false);
   if(zc)
   {
        iofwdutil::mm::NBIOMemoryManager::instance().enableZeroCopy();
   }
   else
   {
        iofwdutil::mm::NBIOMemoryManager::instance().disableZeroCopy();
   }
   iofwdutil::mm::NBIOMemoryManager::instance().start();
}

void IOFWDFrontend::run()
{
   ALWAYS_ASSERT (handler_);

   post_testunexpected ();
}

void IOFWDFrontend::destroy ()
{
   stop_ = true;
   ZLOG_INFO (log_, "Cancelling IOFWDFrontend testunexpected resource call");
   rbmi_.cancel (unexpected_handle_);
}

void IOFWDFrontend::handleIncoming (int count, const BMI_unexpected_info  * info )
{
   ASSERT (count <= BATCH_SIZE);

   Request *  reqs[BATCH_SIZE];
   int ok = 0;

   encoder::xdr::XDRReader reader;

   int32_t opid;

   // Try to do as little as possible, ship everything else to the request
   // queue where it can be handled by a thread
   for (int i=0; i<count; ++i)
   {
      // decode opid and queue the request

      // Request should be at least contain
      if (info[i].size < static_cast<int>( req_minsize_))
      {
         ZLOG_ERROR (log_, format("Received invalid request: "
                  "request size too small (%u < %u). Ignoring")
               % info[i].size % req_minsize_);

         // Create a unexpected buffer and let that one care about freeing
         // that
         bmi::BMIUnexpectedBuffer cleanup (info[i]);

         continue;
      }
      reader.reset (info[i].buffer, info[i].size);
      reader >> opid;

      if (opid < 0 || (opid > static_cast<int>(map_.size())))
      {
         ZLOG_ERROR(log_, format("Invalid request op id (%i) received") % opid);

         // Free buffer and continue
         bmi::BMIUnexpectedBuffer cleanup (info[i]);
         continue;
      }
      else
      {
         // Request now owns the BMI buffer
         ALWAYS_ASSERT(map_[opid]);
         reqs[ok++] = map_[opid] (opid, info[i], res_);
      }
   }
   handler_->handleRequest (ok, &reqs[0]);
}

void IOFWDFrontend::post_testunexpected ()
{
   if (stop_)
      return;

   unexpected_handle_ = rbmi_.post_testunexpected
      (boost::bind (&IOFWDFrontend::newUnexpected,
            boost::ref(*this), _1), info_.size(), &ue_count_, &info_[0],
       std::make_pair(ZOIDFS_BMI_MINTAG, ZOIDFS_BMI_MAXTAG));
}

/**
 * This method is called by iofwdevent::BMIResource when an unexpected message
 * comes in.
 */
void IOFWDFrontend::newUnexpected (iofwdevent::CBException e)
{
   if (!e.hasException ())
   {
      if (ue_count_ != 0)
      {
         handleIncoming (ue_count_, &info_[0]);
      }
   }
   else
   {
      // We only except an exception due to cancelling the unexpected receive.
      if (!e.isCancelled ())
         e.check ();
   }
   // Wait
   if (!stop_)
      post_testunexpected();
}

//===========================================================================
   }
}
