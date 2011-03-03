#ifndef ZOIDFSCLIENT_H
#define ZOIDFSCLIENT_H

void zoidfs_client_swap_addr(BMI_addr_t * naddr);
void zoidfs_client_def_addr(void);
void zoidfs_client_set_def_addr(BMI_addr_t * addr);
bmi_context_id * zoidfs_client_get_context();
void zoidfs_client_set_pipeline_size(size_t psize);
unsigned int zoidfs_xdr_size_processor_cache_init();

#endif
