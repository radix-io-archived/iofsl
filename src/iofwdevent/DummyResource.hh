#ifndef IOFWDEVENT_DUMMYRESOURCE_HH
#define IOFWDEVENT_DUMMYRESOURCE_HH

#include <boost/function.hpp>
#include <vector>
#include <boost/thread.hpp>

#include "Resource.hh"

namespace iofwdevent
{
//==========================================================================

/**
 *  \brief  DummyResource: completes immediately
 *
 *  For testing, a resource that doesn't do anything but calling the 
 *  callback (with success or failure state).
 *
 *  Supports delayed calling.
 *
 */
class DummyResource : public Resource
{
public:
   DummyResource ();

   ~DummyResource ();

   /// Immediately calls the callback with specified status
   void immediate (const CBType & cb, int status);

   /**
    * Puts cb on waiting list; Will complete with specified status when
    * complete is called.
    */
   void defer (const CBType & cb, int status);

   /**
    * Complete all deferred operations.
    */
   void complete ();


   virtual void start ();

   virtual void stop ();

   virtual bool cancel (Handle h);

   virtual bool started () const;

protected:
   std::vector<boost::function<void ()> > deferred_;

   bool started_;

   mutable boost::mutex lock_;
};

//==========================================================================
}

#endif

