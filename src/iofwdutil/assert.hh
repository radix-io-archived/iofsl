#ifndef IOFWDUTIL_ASSERT_HH
#define IOFWDUTIL_ASSERT_HH

/*
 * This header makes 
 *    STATIC_ASSERT
 *    ASSERT
 *    ALWAYS_ASSERT
 * available
 */


#include <cassert>
#include "always_assert.hh"
#include <boost/static_assert.hpp>

#define STATIC_ASSERT(a) BOOST_STATIC_ASSERT(a)
#define ASSERT(a) assert(a)

#endif
