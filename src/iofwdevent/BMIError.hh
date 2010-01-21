#ifndef IOFWDEVENT_BMIERROR_HH
#define IOFWDEVENT_BMIERROR_HH

namespace iofwdevent
{
   //========================================================================
   
   
   /**
    * Given a bmi return code, convert to error string in buf.
    * Will always null-terminate.
    */
   void bmi_strerror_r (int bmiret, char * buf, size_t bufsize);

   //========================================================================
}

#endif
