#ifndef ZOIDFSCLIENT_H
#define ZOIDFSCLIENT_H

#include <bmi.h>
#include "zoidfs/zoidfs.h"

void zoidfs_client_swap_addr(BMI_addr_t * naddr);
void zoidfs_client_def_addr(void);
void zoidfs_client_set_def_addr(BMI_addr_t * addr);
bmi_context_id * zoidfs_client_get_context();
void zoidfs_client_set_pipeline_size(size_t psize);

typedef struct
{
  /* zoidfs_write varibles */
  size_t mem_count;
  void ** mem_starts;
  size_t * mem_sizes;
  size_t file_count;
  zoidfs_file_ofs_t * file_starts;
  zoidfs_file_ofs_t * file_sizes;
  zoidfs_op_hint_t * op_hint;

} zoidfs_write_vars;

typedef struct
{
    const zoidfs_handle_t * handle;
    size_t file_count;
    zoidfs_file_size_t * file_sizes;
    zoidfs_op_hint_t * op_hint;

    void ** read_buf;
    size_t * read_buf_size;
    size_t read_mem_count;
    void ** output_buf;
    size_t * output_sizes;
    size_t output_mem_count;
} zoidfs_read_vars;


#endif
