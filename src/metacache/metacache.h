#ifndef IOFWD_METACACHE_H
#define IOFWD_METACACHE_H

struct metacache; 

typedef struct metacache * metacache_handle;


int metacache_init (metacache_handle * handle);

int metacache_done (metacache_handle * handle);


#endif
