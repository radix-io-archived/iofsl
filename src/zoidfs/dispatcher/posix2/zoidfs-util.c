#include  <stdio.h>
#include <assert.h>
#include <ctype.h>
#include "zoidfs-util.h"

static inline char digit2hex (int num)
{
   assert (num >= 0 && num <= 15); 

   if (num < 10)
      return (char) ('0' + num); 
   return (char) ('a' + (num - 10)); 
}


int zoidfs_handle_to_text (const zoidfs_handle_t *  handle, char * buf, int size)
{
   unsigned char * src= (unsigned char*) handle; 
   unsigned int i; 

   /* verify buffer size */ 
   if ((unsigned int) size < ((sizeof (zoidfs_handle_t)*2)+1))
      return 0; 

   for (i=0; i<sizeof (zoidfs_handle_t); ++i)
   {
      *buf++ = digit2hex ((*src & 0xf0) >> 4); 
      *buf++ = digit2hex (*src &  0x0f); 
      ++src; 
   }
   *buf=0; 
   return 1; 
}

int hex2digit (char d)
{
   d= (char) tolower (d); 
   
   if (d >= '0' && d <= '9')
      return d - '0';
   if (d >= 'a' && d<= 'f')
      return d - 'a' + 10; 
   return -1; 
}

int zoidfs_text_to_handle (const char * buf, zoidfs_handle_t * handle)
{
   unsigned char * dst = (unsigned char*) handle; 
   int remaining = sizeof(zoidfs_handle_t); 

   while (remaining && *buf)
   {
      unsigned char final; 
      int r = hex2digit (*buf++); 
      if (r<0)
         return 0; 
      final = (unsigned char) (r << 4); 
      if (!*buf)
         break; 
      r = hex2digit (*buf++);
      if (r<0)
         return 0; 
      final = (unsigned char) (final | r); 
      *dst++ = final; 
      --remaining; 
   }
   return !remaining; 
}


const char * zoidfs_handle_string (const zoidfs_handle_t * handle)
{
   static char buf[(sizeof(zoidfs_handle_t)*2)+1]; 
   zoidfs_handle_to_text (handle, buf, sizeof(buf));
   return buf; 
}

