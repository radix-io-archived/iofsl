#ifndef TEST_BENCHMARK_TEST
#define TEST_BENCHMARK_TEST
#include <time.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/time.h>
#include <sys/types.h>
#include "mpi.h"
#include "zoidfs/zoidfs.h"
#include "c-util/tools.h"
#include "zoidfs/zoidfs-proto.h"
#include "iofwd_config.h"
#include <unistd.h>
void lookupInput (zoidfs_handle_t * outHandle, char * filename);
void test (char * address, char * config, char * inDataset, char * outDataset, 
           int readSize, int runs);
#endif
