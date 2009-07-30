/*
 * zoidfs-sysio.h
 * SYSIO driver for the ZOIDFS API.
 *
 * Jason Cope <copej@mcs.anl.gov>
 * Nawab Ali <alin@cse.ohio-state.edu>
 * 
 */

#ifndef _ZOIDFS_SYSIO_H_
#define _ZOIDFS_SYSIO_H_

#include "../zint-handler.h"
#include "iofwd_config.h" /* Need this for the SIZEOF defines */

/*
 * Size of the zoidfs handle header and payload
 */
#define ZOIDFS_HANDLE_DATA_SIZE 32
#define ZOIDFS_HANDLE_HEADER_SIZE 4
#define ZOIDFS_HANDLE_PAYLOAD_SIZE (ZOIDFS_HANDLE_DATA_SIZE - ZOIDFS_HANDLE_HEADER_SIZE) 

/* 
 * Size of the struct file_handle_info_export * fhi_export in struct file_handle_info
 */
#ifndef SYSIO_FHE_SIZE
#define SYSIO_FHE_SIZE SIZEOF_STRUCT_FILE_HANDLE_INFO_EXPORT_P 
#endif /* SYSIO_FHE_SIZE */

/* 
 * Size of the size_t fhi_handle_len in struct file_handle_info
 */
#ifndef SYSIO_FHILEN_SIZE
#define SYSIO_FHILEN_SIZE SIZEOF_SIZE_T
#endif /* SYSIO_FHILEN_SIZE */

/* 
 * Since we are using small buffers and to save space in the zoidfs handle,
 * we store the sysio handle length field in the zoidfs handle as a uint8_t 
 */
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
