/*
 * zoidfs-sysio.h
 * SYSIO driver for the ZOIDFS API.
 *
 * Jason Cope <copej@mcs.anl.gov>
 * Nawab Ali <alin@cse.ohio-state.edu>
 *
 */

#ifndef ZOIDFS_SYSIO_H
#define ZOIDFS_SYSIO_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/uio.h>
#include <getopt.h>
#include <sys/statvfs.h>
#include <dirent.h>
#include <errno.h>
#include <pthread.h>

#include "zoidfs/zoidfs.h"
#include "c-util/tools.h"

#if defined(SYSIO_LABEL_NAMES)
#include "sysio.h"
#endif

#include "xtio.h"
#include "fhi.h"

#include "zoidfs/dispatcher/zint-handler.h"

#include "iofwd_config.h" /* Need this for the SIZEOF defines */

/*
 * Size of the zoidfs handle header and payload
 */
#undef ZOIDFS_HANDLE_DATA_SIZE
#define ZOIDFS_HANDLE_DATA_SIZE 32
#undef ZOIDFS_HANDLE_HEADER_SIZE
#define ZOIDFS_HANDLE_HEADER_SIZE 4
#undef ZOIDFS_HANDLE_PAYLOAD_SIZE
#define ZOIDFS_HANDLE_PAYLOAD_SIZE (ZOIDFS_HANDLE_DATA_SIZE - ZOIDFS_HANDLE_HEADER_SIZE)

/*
 * Size of the struct file_handle_info_export * fhi_export in struct file_handle_info
 */
#undef SYSIO_FHE_SIZE
#ifndef SYSIO_FHE_SIZE
#define SYSIO_FHE_SIZE SIZEOF_STRUCT_FILE_HANDLE_INFO_EXPORT_P
#endif /* SYSIO_FHE_SIZE */

/*
 * Size of the size_t fhi_handle_len in struct file_handle_info
 */
#undef SYSIO_FHILEN_SIZE
#ifndef SYSIO_FHILEN_SIZE
#define SYSIO_FHILEN_SIZE SIZEOF_SIZE_T
#endif /* SYSIO_FHILEN_SIZE */

/*
 * Since we are using small buffers and to save space in the zoidfs handle,
 * we store the sysio handle length field in the zoidfs handle as a uint8_t
 */
#undef SYSIO_FHILENPACK
#ifndef SYSIO_FHILENPACK_SIZE
#define SYSIO_FHILENPACK_SIZE SIZEOF_UINT8_T
#endif /* SYSIO_FHILENPACK_SIZE */

/*
 * Size of the char fhi_handle[] in struct file_handle_info. The size of this field is computed
 * by the following equations depending on the system arch...
 *
 * sysio handle data size = (zoidfs_handle_size) - (zoidfs header) - (sysio fhe pointer) - (sysio fh len)
 *                        = 32 - 4 - 8 - 1
 *                        = 19 for 64-bit systems
 *                        or
 *                        = 32 - 4 - 4 - 1
 *                        = 23 for 32-bit systems
 */
#undef SYSIO_HANDLE_DATA_SIZE
#ifndef SYSIO_HANDLE_DATA_SIZE
#define SYSIO_HANDLE_DATA_SIZE (ZOIDFS_HANDLE_DATA_SIZE - ZOIDFS_HANDLE_HEADER_SIZE - SYSIO_FHE_SIZE - SYSIO_FHILENPACK_SIZE)
#endif /* SYSIO_HANDLE_DATA_SIZE */

/*
 * structure with function pointers to the sysio zoidfs functions
 */
extern zint_handler_t sysio_handler;

#endif /*_ZOIDFS_SYSIO_H_ */

/*
 * Local variables:
 *  mode: c
 *  c-indent-level: 4
 *  c-basic-offset: 4
 * End:
 *
 * vim: ft=c ts=4 sts=4 sw=4 expandtab
 */
