#include <bzlib.h>
int bzip_compress (bz_stream * stream, void ** source, size_t * length, void ** dest,
                   size_t * output_length, int close);
int bzip_compress_hook (void * stream, void ** source, size_t * length, void ** dest,
                        size_t * output_length, int close);
int bzip_compress_init (char * type, int block_size, int verbosity, 
                        int work_factor, void ** library);
int bzip_decompress_init (void ** transform);
int bzip_decompress (bz_stream * stream, void ** source,
		     size_t * length, void ** dest,
                     size_t * output_length, int close);
