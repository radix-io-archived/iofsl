#include "ExceptionBacktrace.hh"

#include <cstdlib>
#include <sstream>
#include <cstdio>
#include <boost/utility.hpp>
#include <memory>

#include "iofwd_config.h"

#ifdef HAVE_EXECINFO_H
#include <execinfo.h>

#ifdef HAVE_CXXABI_H
#include <cxxabi.h>
#endif

#endif

namespace iofwdutil
{
   //========================================================================

   ExceptionBacktrace & ExceptionBacktrace::operator = (const
         ExceptionBacktrace & other)
   {
      if (this == boost::addressof (other))
         return *this;

      trace_.reset (new void * [other.count_]);
      count_ = other.count_;

      std::uninitialized_copy (&other.trace_[0], &other.trace_[count_],
            &trace_[0]);
      return *this;
   }

   ExceptionBacktrace::ExceptionBacktrace (const ExceptionBacktrace & other)
      : trace_ (new void * [other.count_]),
        count_ (other.count_)
   {
      std::uninitialized_copy (&other.trace_[0], &other.trace_[count_],
            &trace_[0]);
   }

   ExceptionBacktrace::ExceptionBacktrace ()
      : trace_(0), count_ (0)
   {
#ifdef HAVE_EXECINFO_H
      trace_.reset (new void * [MAXDEPTH]);
      count_ = backtrace (trace_.get(), MAXDEPTH);
#endif
   }

   static std::string demangle (const char * ptr)
   {
#ifndef HAVE_CXXABI_H
      return std::string (ptr);
#else
      int status;
      char temp[128];
      char* demangled;

      if (1 == sscanf(ptr, "%*[^(]%*[^_]%127[^)+]", temp))
      {
         demangled = abi::__cxa_demangle(temp, 0, 0, &status);
         if (demangled)
         {
            std::string result (demangled);
            free (demangled);
            return result;
         }
      }

      return std::string(ptr);
#endif
   }

   std::string ExceptionBacktrace::toString () const
   {
#ifndef HAVE_EXECINFO_H
      ALWAYS_ASSERT(0 == count_);
      return std::string ("backtrace() not supported on this system");
#else
      if (!count_)
         return std::string("backtrace() did not return anything...");

      char ** strings = 0;

      try
      {
         strings = backtrace_symbols (trace_.get(), count_);

         if (!strings)
            return std::string("no backtrace: backtrace_symbols failed...");

         std::ostringstream out;
         out << "backtrace:\n";
         for (int i=0; i<count_; ++i)
         {
            out << "(" << i << ")>>"
               << demangle (strings[i]) << "\n";
         }

         return out.str();
      }
      catch (...)
      {
         free (strings);
         throw;
      }
#endif
   }

   std::string to_string (const ExceptionBacktrace & e)
   {
      return e.toString ();
   }

   //========================================================================
}
