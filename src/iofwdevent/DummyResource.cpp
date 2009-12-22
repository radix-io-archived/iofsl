#include <boost/bind.hpp>

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
   started_ = true;
}

void DummyResource::stop ()
{
   started_= false;
}

bool DummyResource::started () const
{
   return started_;
}

bool DummyResource::cancel (Handle h)
{
   return false;
}

//static void docall (const boost::function<void ()> & cb)
//{ cb(); }

void DummyResource::complete ()
{
   for (size_t i=0; i<deferred_.size(); ++i)
   {
      deferred_[i] ();
   }
   // std::for_each (deferred_.begin (), deferred_.end (), &docall);
   deferred_.clear ();
}

void DummyResource::defer (const CBType & cb, int status)
{
   deferred_.push_back (boost::bind (cb, status));
}

void DummyResource::immediate (const CBType & cb, int status)
{
   cb (status);
}

//===========================================================================
}
