#ifndef IOFWDCLIENT_FAKEBLOCKER_HH
#define IOFWDCLIENT_FAKEBLOCKER_HH

#include "iofwdevent/CBException.hh"
#include "iofwdevent/CBType.hh"

#include <boost/thread.hpp>
#include <boost/bind.hpp>

namespace iofwdclient
{

void iofwdclient_fake_blocker(const iofwdevent::CBType & cb, iofwdevent::CBException e)
{
    fprintf(stderr, "%s:%i enter fake blocking code\n", __func__, __LINE__);
    sleep(1);
    fprintf(stderr, "%s:%i exit fake blocking code\n", __func__, __LINE__);
    cb(e);
    fprintf(stderr, "%s:%i callback invoked\n", __func__, __LINE__);
}

void iofwdclient_fake_blocker_nb(const iofwdevent::CBType & cb, iofwdevent::CBException e)
{
    boost::thread t(boost::bind(iofwdclient_fake_blocker, cb, e));
}

}
#endif
