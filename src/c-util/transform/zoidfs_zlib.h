
int compress_init (char * type, int level, void ** library);
int decompress_init (char * type, void ** library);
int zlib_compress_hook (void * stream, void ** source, size_t * length, void ** dest,
                        size_t * output_length, int close);
int zlib_compress (z_stream * stream, void ** source, size_t * length, void ** dest,
                   size_t * output_length, int close);

