#include "iofwd/TaskPool.hh"
#include <boost/bind.hpp>
#include <boost/function.hpp>

namespace iofwd
{

Task * TaskPool::createTask(int tt)
{
    Task * t = NULL;

    /* given the task type, allocate it */
    /* if type is invalid, return NULL */
    switch(tt)
    {
        case zoidfs::ZOIDFS_PROTO_WRITE:
        {
            t = new WriteTask(bmi_, reschedule_, boost::bind(&iofwd::TaskPool::put, this, _1, _2));
            break;
        }
        case zoidfs::ZOIDFS_PROTO_READ:
        {
            //t = new ReadTask(bmi_, reschedule_, boost::bind(&iofwd::TaskPool::put, this, _1, _2));
            break;
        }
        default:
            break;
    };
    return t;
}

} /* namespace iofwd */
