#ifndef NET_COMMUNICATOR_HH
#define NET_COMMUNICATOR_HH

#include "net/Group.hh"

#include "iofwdutil/IntrusiveHelper.hh"

namespace net
{
   //========================================================================

   /**
    * Communicator is a group that contains the local process
    */
   class Communicator : public iofwdutil::IntrusiveHelper
   {
      public:
         Communicator (ConstGroupHandle group, size_t rank)
           : group_ (group), rank_ (rank)
         {
         }

      public:
         ConstGroupHandle getGroup () const
         { return group_; }

         size_t rank () const
         { return rank_; }

         size_t size () const
         { return group_->size (); }

         ConstAddressPtr operator [] (size_t pos) const
         { return (*group_)[pos]; }

      protected:
         ConstGroupHandle group_;
         size_t rank_;
   };

   INTRUSIVE_PTR_HELPER (Communicator);

   typedef boost::intrusive_ptr<Communicator> CommunicatorHandle;
   typedef boost::intrusive_ptr<const Communicator> ConstCommunicatorHandle;

   //========================================================================
}

#endif
