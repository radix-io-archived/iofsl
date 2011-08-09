#include "src/iofwdutil/IOFSLSignal.hh"

namespace iofwdutil
{

IOFSLSignal::~IOFSLSignal(void)
{
#ifdef IOFWD_SP_USE_PIPES
    close(fd_[0]);
    close(fd_[1]);
#endif
}

}
