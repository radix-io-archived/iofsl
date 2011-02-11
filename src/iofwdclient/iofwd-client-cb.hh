#include "src/iofwdevent/CBException.hh"

/* called multiple times at diff completion stages of the zoidfs op */
typedef boost::function<void(zoidfs_comp_mask_t, iofwdevent::CBException)>
    IOFWDClientCB;

