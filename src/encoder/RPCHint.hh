#ifndef COMMON_RPCHINT
#define COMMON_RPCHINT
#include "EncoderString.hh"
#include "Util.hh"
#include "Size.hh"
#include "EncoderException.hh"
#include "EncoderWrappers.hh"

#include "zoidfs/hints/zoidfs-hints.h"

#include <boost/unordered_map.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/foreach.hpp>
#include <boost/utility/enable_if.hpp>

namespace encoder
{
    typedef boost::unordered_map<std::string, std::string> hash_table;
    class RPCHint
    {
      public:
          RPCHint() 
          {
            len_ = 0;
            maxkeys_ = ZOIDFS_HINT_NKEY_MAX;
            maxlength_ = ZOIDFS_HINT_KEY_MAX;
          }
          void addHint (const std::string key, const std::string value)
          {
            len_++;
            /* Validate that the key does not already exist in hash_table */
            assert(hash_.find(key) == hash_.end());
            assert(value.length() <= maxlength_);

            /* add the key/value pair */
            hash_[key] = value;
          }

          const std::string getHint (const std::string key)
          {
            /* assert that the key is in the hash table */
            assert(hash_.find(key) != hash_.end());            
            return hash_[key];
          }

          size_t getLen () 
          { 
            return len_; 
          }

          void setLen (size_t len) 
          { 
            assert(len_ == 0);            
            len_ = len; 
          }

          boost::shared_ptr<std::string> getKeys ()
          {
            boost::shared_ptr<std::string> keys(new std::string[maxkeys_]);
            size_t count = 0;
            
            BOOST_FOREACH(hash_table::value_type i, hash_) {
                keys.get()[count] = i.first;
                count++;
                assert(count <= maxkeys_);
            }
            return keys;
          }
      private:
          hash_table hash_;
          size_t maxkeys_;
          size_t maxlength_;
          size_t len_;
    };


   template <typename ENC>
   static void process (ENC & e, const RPCHint & p,
         typename only_size_processor<ENC>::type * = 0)
   {
      EncoderString<0,ZOIDFS_HINT_KEY_MAX> hint;
      size_t i = ZOIDFS_HINT_NKEY_MAX;
      process(e, i);
      for (size_t x = 0; x < i; i++)
        process(e,hint);
   }

   template <typename ENC>
   static void process (ENC & e, RPCHint & p,
                        typename only_decoder_processor<ENC>::type * = 0)
   {
      uint32_t len;
      process (e, len);      
      for (size_t x = 0; x < len; x++)
      {      
        EncoderString<0,ZOIDFS_HINT_KEY_MAX> key;
        EncoderString<0,ZOIDFS_HINT_KEY_MAX> value;
        process(e, key);
        process(e, value);
        p.addHint( key.value, value.value);
      }
   }

   template <typename ENC>
   static void process (ENC & e, const RPCHint & p,
                   typename only_encoder_processor<ENC>::type * = 0)
   {
      boost::shared_ptr<std::string> keys = p.getKeys();
      process(e,p.getLen());
      for (size_t x = 0; x < p.getLen(); x++)
      {
        EncoderString<0,ZOIDFS_HINT_KEY_MAX> key(keys.get()[x]);
        EncoderString<0,ZOIDFS_HINT_KEY_MAX> value(p.getHint(key.value));
        process(e,key);
        process(e,value);
      }    
   }
}
#endif
