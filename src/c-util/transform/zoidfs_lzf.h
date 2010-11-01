
int lzf_compress_init (void ** library, size_t buffer_size);
int lzf_compress_hook (void * stream, void ** source, size_t * length, void ** dest,
                        size_t * output_length, int close);
int lzf_decompress_hook (void * state_var, void ** source,
			 size_t * source_len, void ** dest,
			 size_t * dest_len, int close);
int lzf_decompress_init (void ** library);
