#ifndef TEST_BOOST_THREADSAFETY_HH
#define TEST_BOOST_THREADSAFETY_HH

/**
 * Boost.Test is not thread safe.
 * Define thread safe versions of macro's
 */

#include <boost/thread.hpp>

namespace test
{
   namespace boost
   {

      extern ::boost::mutex boost_test_lock_;

   }
}

#define BOOST_CHECK_EQUAL_TS(a,b) \
 { \
    boost::mutex::scoped_lock l__(::test::boost::boost_test_lock_); \
    BOOST_CHECK_EQUAL(a,b); }

#define BOOST_CHECK_MESSAGE_TS(a,b) \
{\
   boost::mutex::scoped_lock l__(::test::boost::boost_test_lock_); \
   BOOST_CHECK_MESSAGE(a,b); \
}

#define BOOST_CHECK_TS(a) \
{\
   boost::mutex::scoped_lock l__(::test::boost::boost_test_lock_);\
   BOOST_CHECK(a);\
}

#define BOOST_TEST_MESSAGE_TS(a) \
{\
   boost::mutex::scoped_lock l__(::test::boost::boost_test_lock_);\
   BOOST_TEST_MESSAGE(a);\
}

#define BOOST_WARN_EQUAL_TS(a,b) \
 { \
    boost::mutex::scoped_lock l__(::test::boost::boost_test_lock_); \
    BOOST_WARN_EQUAL(a,b); }

#define BOOST_WARN_MESSAGE_TS(a,b) \
{\
   boost::mutex::scoped_lock l__(::test::boost::boost_test_lock_); \
   BOOST_WARN_MESSAGE(a,b); \
}

#define BOOST_WARN_TS(a) \
{\
   boost::mutex::scoped_lock l__(::test::boost::boost_test_lock_);\
   BOOST_WARN(a);\
}



#endif
