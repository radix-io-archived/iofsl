
#define BOOST_TEST_MODULE "rpc"
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>


/*
 * goal:
 *
 *    doRPC<func> ( CB, (RPCComm *) channel) (args, args, args, ..., )
 */
