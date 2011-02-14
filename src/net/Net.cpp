#include "Net.hh"

namespace net
{
   //========================================================================

   Net::~Net ()
   {
   }

   void Net::clearAcceptHandler ()
   {
      setAcceptHandler (AcceptHandler ());
   }

   //========================================================================
}
