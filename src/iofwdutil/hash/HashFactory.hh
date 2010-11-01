#ifndef IOFWDUTIL_HASH_HASHFACTORY_HH
#define IOFWDUTIL_HASH_HASHFACTORY_HH

#include <boost/thread.hpp>
#include <map>
#include <boost/iterator/iterator_facade.hpp>
#include <boost/function.hpp>
#include <vector>

#include "HashFunc.hh"
#include "iofwdutil/Singleton.hh"
#include "iofwdutil/tools.hh"

namespace iofwdutil
{
   namespace hash
   {
      //=====================================================================

      /**
       * Factory for hash functions
       *
       * Note: this doesn't use the generic factory since that one doesn't
       * support priorities (yet?)
       */
      class HashFactory : public Singleton<HashFactory>
      {
         public:
            //typedef HashFunc * (*CreateHashFunc) ();
            typedef boost::function<HashFunc * ()> CreateHashFunc;


            /**
             * Will throw NoSuchHashException if no function found.
             */
            HashFunc * getHash (const std::string & name) const;

            HashFunc * getHash (size_t pos) const;

            /** Return all hash functions matching given name. Used for unit
                test. */
            void getHash (const std::string & name, std::vector<HashFunc *> &
                  ret) const;

            /**
             * Higher priority will be selected first.
             */
            void registerHash (const std::string & name, int priority,
                  CreateHashFunc func);


            size_t size () const
            { reghelper(); return map_.size(); }

            HashFactory () : registered_(false)
            { }

         protected:
            typedef struct
            {
               CreateHashFunc func;
               int priority;
            } Entry;
            
            typedef std::multimap<std::string, Entry> FuncMap;

         protected:
            // --- iterator --
            typedef std::pair<std::string, int> hash_iter_value;

            class hash_iterator
               : public boost::iterator_facade<hash_iterator,
               const hash_iter_value, boost::forward_traversal_tag>
            {
               protected:
                  friend class HashFactory;

                  hash_iterator (FuncMap::const_iterator i)
                     : iter_(i), deref_(false)
                  { }

               private:
                  friend class boost::iterator_core_access;

                  void increment ()
                  { ++iter_; deref_ = false; }

               bool equal (hash_iterator const & other) const
               { return other.iter_ == iter_; }

               const hash_iter_value & dereference () const
               {
                  if (!deref_)
                  {
                     value_.first = iter_->first;
                     value_.second = iter_->second.priority;
                     deref_ = true;
                  }
                  return value_;
               }

            private:

               FuncMap::const_iterator iter_;
               mutable hash_iter_value value_;
               mutable bool deref_;
         };

         public:
            // --- iterator support --
            typedef hash_iterator const_iterator;

            const_iterator begin () const
            { reghelper(); return hash_iterator (map_.begin()); }

            const_iterator end () const
            { reghelper (); return hash_iterator (map_.end()); }


         protected:
            // -- helps in static linking with auto registration --
            void reghelper () const;

         protected:
            FuncMap map_;

            mutable boost::mutex lock_;

            mutable bool registered_;
      };


      

      //=====================================================================
   }
}

#endif
