#ifndef IOFWDUTIL_SINGLETON_HPP
#define IOFWDUTIL_SINGLETON_HPP

#include <boost/utility.hpp>
#include <boost/thread/once.hpp>
#include <boost/scoped_ptr.hpp>

namespace iofwdutil
{
//===========================================================================

template<class T>
class Singleton : private boost::noncopyable
{

public:
    static T& instance()
    {
        boost::call_once(init, flag);
        return *t;
    }

    static void init() // never throws
    {
        t.reset(new T());
    }

protected:
    ~Singleton() {}
     Singleton() {}

private:
     static boost::scoped_ptr<T> t;
     static boost::once_flag flag;
};

template<class T> boost::scoped_ptr<T> iofwdutil::Singleton<T>::t(0);

template<class T> boost::once_flag iofwdutil::Singleton<T>::flag = BOOST_ONCE_INIT;

//===========================================================================
}


#endif


