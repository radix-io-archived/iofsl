#ifndef NET_CONNECTION_HH
#define NET_CONNECTION_HH

#include "iofwdevent/ZeroCopyInputStream.hh"
#include "iofwdevent/ZeroCopyOutputStream.hh"

namespace net
{
   //========================================================================

   struct Connection
   {
      iofwdevent::ZeroCopyInputStream *  in;
      iofwdevent::ZeroCopyOutputStream * out;

      Connection () : in(0), out(0) {}

      Connection (iofwdevent::ZeroCopyInputStream * i,
            iofwdevent::ZeroCopyOutputStream * o)
         : in (i), out (o)
      {
      }

   };

   //========================================================================
}

#endif
