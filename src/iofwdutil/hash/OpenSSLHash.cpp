
#include "HashFactory.hh"
#include "OpenSSLHash.hh"
#include "NoSuchHashException.hh"
#include "OpenSSLInit.hh"

namespace iofwdutil
{
   namespace hash
   {
      //=====================================================================

      void OpenSSLHash::error (int )
      {
         // TODO: get proper errors from OpenSSL
         throw ZException ("OpenSSLHash error!");
      }

      OpenSSLHash::OpenSSLHash (const char * name, const EVP_MD * md)
         : md_(md), name_(name)
      {
         // Init openssl if needed
         OpenSSLInit::instance ();

         md_ = EVP_get_digestbyname(name);
         if(!md_)
         {
            throw NoSuchHashException (name);
         }
         EVP_MD_CTX_init(&mdctx_);

         reset();
      }

      OpenSSLHash::~OpenSSLHash ()
      {
         // destructors should never throw. Just log error
         if (EVP_MD_CTX_cleanup(&mdctx_) != 1)
         {
            //ZLOG_ERROR("Error in OpenSSLHash destructor!");
         }
      }

      void OpenSSLHash::reset ()
      {
         check(EVP_DigestInit_ex(&mdctx_, md_, 0));
      }

      std::string OpenSSLHash::getName () const
      {
         return name_;
      }

      size_t OpenSSLHash::getHash (void * dest, size_t bufsize, bool finalize)
      {
         EVP_MD_CTX copy;
         EVP_MD_CTX * ctx;
         if (finalize)
         {
            ctx = &mdctx_;
         }
         else
         {
            EVP_MD_CTX_init (&copy);
            check(EVP_MD_CTX_copy_ex (&copy, &mdctx_));
            ctx = &copy;
         }
      
         unsigned int size = bufsize;
         check(EVP_DigestFinal_ex (ctx, static_cast<unsigned char*>(dest),
                  &size));
         return size;
      }

      size_t OpenSSLHash::getHashSize () const
      {
         return EVP_MD_size(md_);
      }

      void OpenSSLHash::process (const void * d, size_t bytes)
      {
         check (EVP_DigestUpdate (&mdctx_, d, bytes));
      }

      // ------------- registration stuff -----------------------------------

      struct HashType
         {
            const char * name;
            const EVP_MD * md;
         };


      static HashType reglist [] =
                {
                   { "sha1", EVP_sha1() },
                   { "md5", EVP_md5() }
                };


      static HashFunc * createOpenSSLHash (size_t pos)
      {
         const HashType & h = reglist[pos];
         return new OpenSSLHash (h.name, h.md);
      }


      void OpenSSLHash::registerHash ()
      {
         for (size_t i=0; i<sizeof(reglist)/sizeof(reglist[0]); ++i)
         {
            // OpenSSL hash higher priority than default (0)
            HashFactory::instance().registerHash (reglist[i].name, 6,
                  boost::bind(&createOpenSSLHash, i));
         }
      }
      

      //=====================================================================
   }
}
