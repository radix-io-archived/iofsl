#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <boost/array.hpp>
#include <iostream> 
#include <unistd.h>
#include <csignal>
#include "iofwdutil/assert.hh"
#include "iofwdutil/bmi/BMI.hh"
#include "IOFWDFrontend.hh"
#include "iofwdutil/xdr/XDRReader.hh"
#include "iofwdutil/bmi/BMIUnexpectedBuffer.hh"
#include "iofwdutil/xdr/XDRSizeProcessor.hh"
#include "zoidfs/util/zoidfs-wrapped.hh"
#include "zoidfs/zoidfs-proto.h"
#include "iofwdutil/IOFWDLog.hh"

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

using namespace iofwdutil::bmi; 
using namespace iofwdutil;
using namespace zoidfs; 

namespace iofwd
{
   namespace frontend
   {

//===========================================================================

// Anonymous namespace 
namespace 
{
//===========================================================================

class IOFW
{
public:
   IOFW (IOFWDFrontend & fe, iofwdutil::completion::BMIResource & bmires);

   void run ();

   void setHandler (RequestHandler * h) { handler_ = h; }

   // Called when we should stop...
   void destroy ();

   ~IOFW (); 

protected:
   enum { 
      BATCH_SIZE = 64, // The number of unexpected requests we accept
      MAX_IDLE   = 1000 // How long to wait for a request (ms)
   }; 

   void handleIncoming (int count, const BMI_unexpected_info * info); 

protected:
   IOFWDFrontend & fe_; 
   iofwdutil::bmi::BMI & bmi_; 
   iofwdutil::bmi::BMIContextPtr bmictx_; 

   

private:
   // We put the structure here not to burden the stack
   // The front-end is singlethreaded anyway
   BMI_unexpected_info info_[BATCH_SIZE]; 

   volatile sig_atomic_t stop_; 

   RequestHandler * handler_; 

   
protected:
   size_t req_minsize_;         // minimum size of valid incoming request
   
   IOFWDLogSource & log_; 
   
   iofwdutil::completion::BMIResource & bmires_; 
   
};

// ==========================================================================
// map op id to IOFWDRequest

typedef Request * (*mapfunc_t) ( iofwdutil::bmi::BMIContext & bmi,
      int i, const BMI_unexpected_info & info,
      iofwdutil::completion::BMIResource & res);

template <typename T>
static inline Request * newreq (iofwdutil::bmi::BMIContext & bmi,
      int i, const BMI_unexpected_info & info,
      iofwdutil::completion::BMIResource & res)
{ return new T (bmi, i, info, res ) ; }

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

IOFW::IOFW (IOFWDFrontend & fe, iofwdutil::completion::BMIResource & res)
   : fe_ (fe), bmi_ (iofwdutil::bmi::BMI::get()),
   stop_ (false),
   handler_(NULL),
   req_minsize_(iofwdutil::xdr::getXDRSize (uint32_t ()).actual),
   log_ (IOFWDLog::getSource ("iofwdfrontend")),
   bmires_ (res)
{
  
   // Make sure we have a context open
    bmictx_ = bmi_.openContext (); 
}

IOFW::~IOFW ()
{
}

void IOFW::handleIncoming (int count, const BMI_unexpected_info  * info )
{
   ASSERT (count <= BATCH_SIZE); 

   Request *  reqs[BATCH_SIZE]; 
   int ok = 0; 
   iofwdutil::xdr::XDRReader reader; 

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
         reqs[ok++] = map_[opid] (*bmictx_.get(), opid, info[i], bmires_); 
      }
   }
   handler_->handleRequest (ok, &reqs[0]); 
}

void IOFW::run ()
{
   ZLOG_INFO(log_, "IOFW thread running"); 

   // Wait 
   while (!stop_)
   {
      const int count = bmi_.testUnexpected (BATCH_SIZE, &info_[0],MAX_IDLE);
      if (count)
         handleIncoming (count, &info_[0]); 
   }
}

void IOFW::destroy ()
{
   ZLOG_DEBUG(log_, "Notifying listening thread to shut down"); 
   stop_ = true; 
}

//===========================================================================
}


//===========================================================================
//===========================================================================
//===========================================================================

IOFWDFrontend::IOFWDFrontend (iofwdutil::completion::BMIResource & res)
   : log_(IOFWDLog::getSource ("iofwdfrontend")),
   bmires_ (res)
{
}

IOFWDFrontend::~IOFWDFrontend ()
{
   ZLOG_INFO (log_, "Shutting down BMI..."); 
   if (BMI::isCreated ())
      BMI::get().finalize (); 
}

void IOFWDFrontend::init ()
{
   ZLOG_DEBUG (log_, "Initializing BMI"); 

   char * ion_name = getenv("ZOIDFS_ION_NAME");
   if (ion_name == NULL) {
     ZLOG_ERROR (log_, format("ZOIDFS_ION_NAME is empty"));
     exit(-1);
   }

   // IOFW uses bmi, so we need to supply init params here
   ZLOG_INFO (log_, format("Server listening on %s") % ion_name); 
   //BMI::setInitServer ("tcp://127.0.0.1:1234");
   BMI::setInitServer (ion_name);

   // initialize BMI
   IOFW * o = new IOFW (*this, bmires_);
   impl_ = o;
}

void IOFWDFrontend::run()
{
   ALWAYS_ASSERT (impl_);
   IOFW * o = (IOFW *) impl_;
   ALWAYS_ASSERT (handler_);
   o->setHandler (handler_);
   implthread_.reset (new boost::thread(boost::bind (&IOFW::run , o)));
}

void IOFWDFrontend::destroy ()
{
   static_cast<IOFW *> (impl_)->destroy (); 
   
   ZLOG_INFO (log_, "Waiting for IOFWD frontend to shut down..."); 
   implthread_->join (); 
   implthread_.reset (); 
   delete (static_cast<IOFW*> (impl_)); 
}

//===========================================================================
   }
}
