
int compress_init (char * type, int level, void ** library);
int zlib_decompress_init (void ** transform);
int zlib_compress_hook (void * stream, void ** source, size_t * length, void ** dest,
                        size_t * output_length, int close);
int zlib_compress (z_stream * stream, void ** source, size_t * length, void ** dest,
                   size_t * output_length, int close);

int zlib_decompress (z_stream * stream, void ** source,
		     size_t * length, void ** dest,
		     size_t * output_length, int close);
