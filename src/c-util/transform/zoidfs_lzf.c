#include "zoidfs_transform.h"
typedef struct
{
    void * buf;
    void * cur_position;
    void * buf_size;
} lzf_state;
static size_t cur_buf_size;
int lzf_compress_init (void ** library, size_t buffer_size)
{
    lzf_state * tmp = malloc(sizeof(lzf_state));
    (*tmp).buf = malloc(buffer_size);
    (*tmp).cur_position = 0;
    (*tmp).buf_size = buffer_size;

    (*library) = tmp;     
    return 0;
}

int lzf_compress_hook (void * stream, void ** source, size_t * length, void ** dest,
                        size_t * output_length, int close)
{
    return lzf_compress_data((lzf_state *)stream, source, length, dest, output_length, close);
}
int lzf_compress_data (lzf_state * stream, void ** source, size_t * length, void ** dest,
                       size_t * output_length, int close)
{
   int ret;    
   XDR val;
   /* Check to see if there is anything in the buffer to write out */
   if ((*tmp).buffer_size > 0)
   {
        if ((*length) < (*tmp).buffer_size)
        {
            return ZOIDFS_BUF_ERROR;
        }
        memcpy ((*dest),(*tmp).buf,(*tmp).buffer_size);
        (*tmp).buffer_size = 0;
   }
   while((*output_length) >= 5 && (*length) > 0)
   {
        ret = lzf_compress ((*source), (*tmp).buffer_size, (*tmp).buf, (*tmp).buffer_size);
        /* If the compressed size is larger then the uncompressed size */        
        if (ret == 0)
        {
            /* Copy to the internal buffer */
            memcpy((*tmp).buf,(*source),(*tmp).buffer_size);
            ret = (*tmp).buffer_size;
            memcpy((*dest),(void *)'U',1);
        } 
        else
            memcpy((*dest),(void *)'C',1);
        /* move the source pointer forward */
        (*source) += (*tmp).buffer_size;
        (*dest) += 1;
        /* Encode the length of the compressed data */
        xrd_int(&val,ret);
        memcpy((*dest),(void *)val, 4);
        (*dest) += 4;
        (*output_length) += 5;
        /* If there is enough room for the full output write it */
        if ((*output_length) >= ret)
        {
            memcpy((*dest),(*tmp).buf,ret);
            (*output_length) -= ret;        
        }
        /* else write as much as you can then return */
        else
        {
            memcpy((*dest),(*tmp).buf,(*output_length));
            (*tmp).buf += (*output_length);
            (*tmp).buffer_size -= (*output_length);
            (*output_length) = 0;
            return ZOIDFS_BUF_ERROR;
        }
    }
}


}
