#include "src/iofwdutil/SelfPipe.hh"

namespace iofwdutil
{

SelfPipe::~SelfPipe(void)
{
#ifdef IOFWD_SP_USE_PIPES
    close(fd_[0]);
    close(fd_[1]);
#endif
}

}
