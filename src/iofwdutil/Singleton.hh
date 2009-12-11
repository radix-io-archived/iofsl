#ifndef IOFWDUTIL_SINGLETON_HPP
#define IOFWDUTIL_SINGLETON_HPP

#include <boost/utility.hpp>
#include <boost/scoped_ptr.hpp>


#ifdef USE_OWN_SINGLETON
#include <csignal>
#include <boost/thread.hpp>
#else
#include <boost/thread/once.hpp>
#endif

namespace iofwdutil
{
//===========================================================================

template<class T>
class Singleton : private boost::noncopyable
{
#ifdef USE_OWN_SINGLETON
protected:
   enum { NOT_INITIALIZED = 0,
          INITIALIZED     = 2 };
#endif

public:

#ifdef USE_OWN_SINGLETON
   static void build ()
   {
      boost::mutex::scoped_lock l(lock_);
      // Check again in case more than one thread made it into build.
      if (status_ != INITIALIZED)
         return;

      init ();
      status_ = INITIALIZED;
   }

    static T& instance()
    {
       if (status_ != INITIALIZED)
          build ();

       return *t;
    }
#else
 static T& instance()
    {
        boost::call_once(init, flag);
        return *t;
    }
#endif

    static void init() // never throws
    {
        t.reset(new T());
    }

protected:
    ~Singleton() {}
     Singleton() {}

private:
     static boost::scoped_ptr<T> t;

#ifdef USE_OWN_SINGLETON
     static sig_atomic_t status_;
     static boost::mutex lock_;
#else
  static boost::once_flag flag;
#endif
};

template<class T> boost::scoped_ptr<T> iofwdutil::Singleton<T>::t(0);

#ifdef USE_OWN_SINGLETON
template<class T> boost::mutex iofwdutil::Singleton<T>::lock_;
template<class T> sig_atomic_t iofwdutil::Singleton<T>::status_ =
                           iofwdutil::Singleton<T>::NOT_INITIALIZED;
#else
template<class T> boost::once_flag iofwdutil::Singleton<T>::flag = BOOST_ONCE_INIT;
#endif


//===========================================================================
}


#endif


