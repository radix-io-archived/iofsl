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
#include "common/zoidfs-wrapped.hh"

#include "IOFWDLookupRequest.hh"

using namespace iofwdutil::bmi; 

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
   IOFW (IOFWDFrontend & fe);

   void run ();

   // Called when we should stop...
   void destroy ();

   ~IOFW (); 

protected:
   enum { 
      BATCH_SIZE = 64, // The number of unexpected requests we accept
      MAX_IDLE   = 10000 // How long to wait for a request
   }; 

   void handleIncoming (int count, const BMI_unexpected_info * info); 

protected:
   IOFWDFrontend & fe_; 
   iofwdutil::bmi::BMI & bmi_; 

private:
   // We put the structure here not to burden the stack
   BMI_unexpected_info info_[BATCH_SIZE]; 

   volatile sig_atomic_t stop_; 

   typedef Request * (*opidfunc_t) (int opid, const BMI_unexpected_info &
         info); 
};

template <class T> 
inline Request * newrequestfunc (int opid, const BMI_unexpected_info & info)
{ return new  T (opid, info); }



IOFW::IOFW (IOFWDFrontend & fe)
   : fe_ (fe), bmi_ (iofwdutil::bmi::BMI::get()),
   stop_ (false)
{
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
   const unsigned int minsize = iofwdutil::xdr::getXDRSize (opid).actual; 

   // Try to do as little as possible, ship everything else to the request
   // queue where it can be handled by a thread
   for (int i=0; i<count; ++i)
   {

      // decode opid and queue the request

      // Request should be at least contain 
      if (info[i].size < minsize)
      {
         // TODO log and throw
         ALWAYS_ASSERT(false); 
         continue; 
      }
      reader.reset (info[i].buffer, info[i].size); 
      reader >> opid; 
      
      opidfunc_t createfunc = 0; 
      switch (opid)
      {
         case ZOIDFS_PROTO_LOOKUP:
            createfunc = &newrequestfunc<IOFWDLookupRequest>; 
            break;
      default:
         // TODO log and throw
         ALWAYS_ASSERT(false); 
      }

      reqs[ok++] = createfunc (opid, info[i]); 
   }
}

void IOFW::run ()
{
   // TODO: make sure we don't leak memory for invalid requests
   std::cout << "IOFW thread running" << std::endl; 

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
   stop_ = true; 
}

//===========================================================================
}


//===========================================================================
//===========================================================================
//===========================================================================

IOFWDFrontend::IOFWDFrontend ()
{
}

IOFWDFrontend::~IOFWDFrontend ()
{
}

void IOFWDFrontend::init ()
{
   IOFW * o = new IOFW (*this); 
   implthread_.reset (new boost::thread(boost::bind (&IOFW::run , o))); 
   impl_ = o; 
}

void IOFWDFrontend::destroy ()
{
   static_cast<IOFW *> (impl_)->destroy (); 
   implthread_->join (); 
   implthread_.reset (); 
   delete (static_cast<IOFW*> (impl_)); 

}

//===========================================================================
   }
}
