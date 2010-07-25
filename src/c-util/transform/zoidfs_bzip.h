#include <bzlib.h>

int bzip_compress (z_stream * stream, void ** source, size_t * length, void ** dest,
                   size_t * output_length, int close);
int bzip_compress_hook (void * stream, void ** source, size_t * length, void ** dest,
                        size_t * output_length, int close);
int bzip_compress_init (void ** library, size_t total_in);
