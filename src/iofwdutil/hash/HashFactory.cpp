#include <algorithm>
#include <boost/format.hpp>
#include <iterator>

#include "iofwd_config.h"

#include "HashFactory.hh"
#include "NoSuchHashException.hh"
#include "iofwdutil/assert.hh"

#include "SHA1Simple.hh"
#include "NoneHash.hh"

#ifdef HAVE_OPENSSL
#include "OpenSSLHash.hh"
#endif

#ifdef HAVE_ZLIB
#include "Adler32.hh"
#include "CRC32.hh"
#endif

using boost::format;

namespace iofwdutil
{
   namespace hash
   {

      void HashFactory::reghelper () const
      {
         if (registered_)
            return;

         registered_ = true;

         // List the av

         // disabled until bug fixed
         // HASHFUNC_REFERENCE(SHA1Simple);

         HASHFUNC_REFERENCE(NoneHash);
#ifdef HAVE_OPENSSL
         OpenSSLHash::registerHash ();
#endif
#ifdef HAVE_ZLIB
         HASHFUNC_REFERENCE(Adler32);
         HASHFUNC_REFERENCE(CRC32);
#endif
      }

      HashFunc * HashFactory::getHash (size_t pos) const
      {
         reghelper ();

         boost::mutex::scoped_lock l(lock_);

         FuncMap::const_iterator I = map_.begin();
         if (pos >= map_.size())
         {
            throw NoSuchHashException (str(format("Invalid hash func number: %i")
                  % pos));
         }
         advance (I, pos);
         ASSERT(I!=map_.end());
         return I->second.func();
      }
            
      void  HashFactory::getHash (const std::string & name,
            std::vector<HashFunc *> & ret) const
      {
         ret.clear ();
         reghelper ();

         boost::mutex::scoped_lock l(lock_);

         try
         {
            std::pair<FuncMap::const_iterator,FuncMap::const_iterator> R =
               map_.equal_range (name);
            if (R.first == R.second)
            {
               throw NoSuchHashException (name);
            }
            FuncMap::const_iterator I = R.first;
            while (I != R.second)
            {
               ret.push_back (I->second.func());
               ++I;
            }
         }
         catch (...)
         {
            // On error, cleanup return array
            for (size_t i=0; i<ret.size(); ++i)
            {
               delete (ret[i]);
            }
            ret.clear();
            throw;
         }
      }

      HashFunc * HashFactory::getHash (const std::string & name) const
      {
         reghelper ();

         boost::mutex::scoped_lock l(lock_);

         std::pair<FuncMap::const_iterator,FuncMap::const_iterator> R = map_.equal_range (name);
         if (R.first == R.second)
         {
            throw NoSuchHashException (name);
         }
         FuncMap::const_iterator best = R.first;
         FuncMap::const_iterator I = R.first;
         ++I;
         while (I != R.second)
         {
            if (I->second.priority > best->second.priority)
            {
               best = I;
            }
            ++I;
         }
         ASSERT(best!=map_.end());
         return best->second.func ();
      }


      void HashFactory::registerHash (const std::string & name, int priority,
            CreateHashFunc func)
      {
         boost::mutex::scoped_lock l(lock_);

         Entry e;
         e.func = func;
         e.priority = priority;

         map_.insert (std::make_pair (name, e));
      }


   }
}
