#ifndef IOFWDUTIL_FACTORY_HH
#define IOFWDUTIL_FACTORY_HH

#include <boost/thread.hpp>
#include <algorithm>
#include <map>
#include "Singleton.hh"
#include "FactoryHelper.hh"
#include "FactoryException.hh"

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
 *         typedef boost::mpl::list<P1,P2,...,PN> FACTORY_SIGNATURE;
 *
 *         where P1..Pn are the types of the parameters to the BASE
 *         constructor.
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
 *        typedef boost::mpl::list<size_t> FACTORY_SIGNATURE;
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
            throw FactoryException ("Unknown key in Factory::construct!");
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
