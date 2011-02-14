#ifndef NET_GROUP_HH
#define NET_GROUP_HH

#include "iofwdutil/IntrusiveHelper.hh"
#include "net/Address.hh"

#include <vector>
#include <boost/intrusive_ptr.hpp>

namespace net
{
   //========================================================================


   /**
    * Ordered Collection of addresses
    */
   class Group : public iofwdutil::IntrusiveHelper
   {
      public:
         void push (const AddressPtr & n)
         { addrs_.push_back (n); }

      public:
         size_t size () const;

         ConstAddressPtr operator [] (size_t pos) const
         { return ConstAddressPtr (addrs_[pos].get()); }

      protected:
         std::vector<AddressPtr> addrs_;
   };

   INTRUSIVE_PTR_HELPER (Group);

   typedef boost::intrusive_ptr<Group> GroupHandle;
   typedef boost::intrusive_ptr<const Group> ConstGroupHandle;

   //========================================================================
}

#endif
