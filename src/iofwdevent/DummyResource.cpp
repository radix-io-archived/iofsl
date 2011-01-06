#include <boost/bind.hpp>

#include "iofwdutil/tools.hh"
#include "DummyResource.hh"

namespace iofwdevent
{
//===========================================================================

DummyResource::DummyResource ()
   : started_(false)
{
}

DummyResource::~DummyResource ()
{
}

void DummyResource::start ()
{
   boost::mutex::scoped_lock l(lock_);

   started_ = true;
}

void DummyResource::stop ()
{
   boost::mutex::scoped_lock l(lock_);

   started_= false;
}

bool DummyResource::started () const
{
   boost::mutex::scoped_lock l(lock_);

   return started_;
}

bool DummyResource::cancel (Handle UNUSED(h))
{
   boost::mutex::scoped_lock l(lock_);

   return false;
}

//static void docall (const boost::function<void ()> & cb)
//{ cb(); }

void DummyResource::complete ()
{
   boost::mutex::scoped_lock l(lock_);

   for (size_t i=0; i<deferred_.size(); ++i)
   {
      deferred_[i] ();
   }
   // std::for_each (deferred_.begin (), deferred_.end (), &docall);
   deferred_.clear ();
}

void DummyResource::defer (const CBType & cb, CBException e)
{
   boost::mutex::scoped_lock l(lock_);

   deferred_.push_back (boost::bind (cb, e));
}

void DummyResource::immediate (const CBType & cb, CBException e)
{
   // no need for lock

   cb (e);
}

//===========================================================================
}
