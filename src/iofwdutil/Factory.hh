#ifndef IOFWDUTIL_FACTORY_HH
#define IOFWDUTIL_FACTORY_HH

#include "FactoryHelper.hh"
#include "FactoryException.hh"
#include "Singleton.hh"
#include "FactoryClient.hh"

#include <boost/thread.hpp>
#include <algorithm>
#include <map>

namespace iofwdutil
{
//===========================================================================
/**
 * Generic Factory:
 *
 *    The factory constructs classes of type BASE (or derivatives).
 *    Each registered type has a KEY, used in specifying which type needs
 *    to be constructed.
 *
 *    Factory expects BASE to have the following properties:
 *       
 *     
 *         A macro listing the constructor parameters for this factory base
 *         type.
 *
 *         FACTORY_CONSTRUCTOR_PARAMS(P1,...,Pn);
 *
 *         where P1..Pn are the types of the parameters to the BASE
 *         constructor.
 *
 *         FACTORY_CONSTRUCTOR_PARAMS is defined in FactoryClient.hh
 *
 *    Automatic registration can be done by using 
 *
 *    FACTORYAUTOREGISTER(KEYTYPE,BASETYPE,YOURTYPE,YOURKEYVAL);
 *
 *    (FACTORYAUTOREGISTER_TAG(KEYTYPE,BASETYPE,TAG,YOURTYPE,YOURKEYVAL) for
 *    non-default tags.
 *
 *    For example:
 *
 *    struct Geometric
 *    {
 *    };
 *
 *    struct Circle : public Geometric
 *    {
 *        FACTORY_CONSTRUCTOR_PARAMS(unsigned int);
 *
 *        Circle (unsigned int radius)
 *        {
 *        ...
 *        }
 *    };
 *
 *    FACTORYAUTOREGISTER(std::string,Geometric,Circle,"CIRCLE");
 *
 *    Factory<std::string,Geometric>::construct ("CIRCLE")(3);
 *
 *    See linker notes in FactoryAutoRegister.
 *
 *
 *    The TAG argument can be used to create multiple factories serving the
 *    same interface. For example, for GenericTransform this enables us to
 *    have an 'encode' and 'decode' factory.
 */
template <typename KEY, typename BASE, typename TAG = void>
struct Factory : public Singleton<Factory<KEY,BASE,TAG> >
{
   typedef typename FactoryHelper<BASE>::CONSTFUNC CONSTFUNC;
   public:

      static CONSTFUNC construct (const KEY & key)
      { return Factory<KEY,BASE,TAG>::instance().constructHelper (key); }

      CONSTFUNC constructHelper(const KEY & key) const
      {
         boost::mutex::scoped_lock l (lock_);

         typename ContainerType::const_iterator I = map_.find (key);
         if (I == map_.end())
         {
            ZTHROW (NoSuchFactoryKeyException ());
         }
         return I->second;
      }

      CONSTFUNC operator () (const KEY & key) const
      { return construct(key); }

      void add (const KEY & name, CONSTFUNC func)
      {
         boost::mutex::scoped_lock l(lock_);
         map_.insert (std::make_pair (name, func));
      }

      size_t size() const
      { return map_.size(); }


      /**
       * Output all the registered keys to the output iterator specified
       */
      template <typename OUT>
      void keys (OUT out) const
      {
         typename ContainerType::const_iterator i1 (map_.begin());
         typename ContainerType::const_iterator i2 (map_.end());
         while (i1 != i2)
         {
            *out = i1->first;
            ++out;
            ++i1;
         }
      }

      Factory ()
      {
      }

      ~Factory ()
      {
      }

   protected:
      typedef std::map<KEY, CONSTFUNC> ContainerType;
      ContainerType map_;

      mutable boost::mutex lock_;
};

//===========================================================================
}

#endif
