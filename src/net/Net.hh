#ifndef NET_NET_HH
#define NET_NET_HH

#include "Net-fwd.hh"
#include "iofwdevent/ZeroCopyInputStream.hh"
#include "iofwdevent/ZeroCopyOutputStream.hh"
#include "Connection.hh"
#include "Address.hh"
#include "Group.hh"

#include <vector>
#include <boost/function.hpp>


namespace net
{
   //========================================================================

   /**
    * Network abstraction, stream oriented.
    * Created as a basis for RPC
    */
   class Net
   {
      public:

         typedef iofwdevent::ZeroCopyInputStream ZeroCopyInputStream;
         typedef iofwdevent::ZeroCopyOutputStream ZeroCopyOutputStream;

         // ------------------------------------------------------
         // ---- Address service ---------------------------------
         // ------------------------------------------------------
         
         virtual void lookup (const char * location, AddressPtr * ptr,
               const iofwdevent::CBType & cb) = 0;


         // ------------------------------------------------------
         // ---- Registering handler for incoming connections ----
         // ------------------------------------------------------

         struct AcceptInfo
         {
            ConstAddressPtr source;
            ZeroCopyInputStream  * in;
            ZeroCopyOutputStream * out;
         };

         typedef boost::function<void (const AcceptInfo & info)>
            AcceptHandler;

         virtual void setAcceptHandler (const AcceptHandler & handler) = 0;

         void clearAcceptHandler ();

         // ------------------------------------------------------
         // --- Making connections -------------------------------
         // ------------------------------------------------------

         /**
          * Make an outgoing connection to address.
          * Returns input and output stream.
          *
          * Ownership is transferred, so the caller is expected to close the
          * streams (at any time).
          *
          * Could add hints here to make it possible to set the max packet
          * size for example (to improve memory efficiency).
          * Actually, could be a send packet and receive
          * packet size.
          */
         virtual Connection connect (const ConstAddressPtr & addr) = 0;


         // ------------------------------------------------------
         // --- Groups -------------------------------------------
         // ------------------------------------------------------

         virtual void createGroup (GroupHandle * group,
               const std::vector<std::string> & members,
               const iofwdevent::CBType & cb) = 0;

         virtual ~Net ();
   };

   //========================================================================
}

#endif
