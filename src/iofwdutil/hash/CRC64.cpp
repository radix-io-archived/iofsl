#include "iofwdutil/Singleton.hh"
#include "iofwdutil/tools.hh"
#include "iofwdutil/assert.hh"

#include "CRC64.hh"

#include <vector>

#define POLY64REV     0x95AC9329AC4BC9B5ULL
#define INITIALCRC    0xFFFFFFFFFFFFFFFFULL

namespace
{
   // Anonymous namespace for CRC table

   class CRCTable : public iofwdutil::Singleton<CRCTable>
   {
      public:

         CRCTable ()
            : table_(256)
         {
            init_table ();
         }

         uint64_t operator [] (size_t i) const
         { return table_[i]; }

      protected:

         // Initialize the CRC table.
         void init_table()
         {
            table_.resize (256);
            for (unsigned int i = 0; i < table_.size(); i++)
            {
               uint64_t part = i;
               for (unsigned int j = 0; j < 8; j++)
               {
                  if (part & 1)
                     part = (part >> 1) ^ POLY64REV;
                  else
                     part >>= 1;
               }
               table_[i] = part;
            }
         }

      protected:
         std::vector<uint64_t> table_;

   };

}


namespace iofwdutil
{
   namespace hash
   {
      //=======================================================================

      HASHFUNC_AUTOREGISTER(CRC64, "crc64", 1);

      //=======================================================================

      CRC64::CRC64 ()
         : state_(INITIALCRC)
      {
      }

      std::string CRC64::getName () const
      {
         return "CRC64";
      }

      void CRC64::reset ()
      {
         state_ = INITIALCRC;
      }

      void CRC64::process (const void * d, size_t size)
      {
         CRCTable & table_ = CRCTable::instance ();

         const unsigned char * cur = static_cast<const unsigned char *>(d);
         const unsigned char * end = cur + size;
         while (cur < end)
            state_ = table_[(state_ ^ *cur++) & 0xff] ^ (state_ >> 8);
      }

      size_t CRC64::getHash (void * dest, size_t bufsize,
            bool UNUSED(finalize))
      {
         ALWAYS_ASSERT(bufsize >= sizeof(uint64_t));
         *static_cast<uint64_t*>(dest) = state_;
         return sizeof(uint64_t);
      }

      size_t CRC64::getHashSize () const
      {
         return sizeof(uint64_t);
      }

      //=======================================================================
   }
}
