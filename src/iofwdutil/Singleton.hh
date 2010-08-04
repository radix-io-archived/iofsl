#ifndef IOFWDUTIL_SINGLETON_HPP
#define IOFWDUTIL_SINGLETON_HPP

#include <boost/utility.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/thread/once.hpp>

#include <cassert>

namespace iofwdutil
{
//===========================================================================

/**
 * Usage:
 *   Derive the class you want to make a singleton from Singleton<CLASS>
 *   i.e.
 *
 *   class MySingleton : public Singleton<MySingleton>
 *   {
 *     ...
 *
 *   This injects a static 'instance()' method in the class which can
 *   be used to obtain a reference to the singleton instance.
 *   
 *   Note: Singleton is threadsafe.
 *
 *   @TODO Add valgrind ignore macro for memory allocated here.
 *
 *   @TODO Singleton cleanup possible if, when creating the singleton, we add
 *   register a delete method that we manually call when shutting down.
 */
template<class T>
class Singleton : private boost::noncopyable
{
public:

 static T& instance()
    {
        boost::call_once(init, flag);
        assert (t);
        return *t;
    }

    static void init() // never throws
    {
       assert (t == 0);
       t = new T();
    }

protected:
    ~Singleton()
    {
    }

     Singleton() {}

private:
     static T * t;
     static boost::once_flag flag;
};

/*
 * We cannot use a scoped_ptr or any other non-pod type for T or
 * boost::once_flag because otherwise the initialization function for that
 * type might be called after the singleton was initialized.
 *
 * i.e. if the singleton is used from within a function keyed to global
 * constructors of static variables, the singleton might access 't' or 'flag'
 * without it being initialized. 
 *
 * In addition, if later the initialization function for 't' or 'flag' runs,
 * it would overwrite their value.
 */

/*
Objects with static storage duration (3.7.1) shall be zero-initialized (8.5)
before any other initialization takes place. Zero-initialization and
initialization with a constant expression are collectively called static
initialization; all other initialization is dynamic initialization. Objects of
POD types (3.9) with static storage duration initialized with constant
expressions (5.19) shall be initialized before any dynamic initialization
takes place. Objects with static storage duration defined in namespace scope
in the same translation unit and dynamically initialized shall be initialized
in the order in which their definition appears in the translation unit.
*/
template<class T> T * iofwdutil::Singleton<T>::t;
template<class T> boost::once_flag iofwdutil::Singleton<T>::flag = BOOST_ONCE_INIT;


//===========================================================================
}


#endif


