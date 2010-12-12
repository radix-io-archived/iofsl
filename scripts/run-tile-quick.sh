#!/bin/bash

IOFSL_DIR="."
OUTFILE="/tmp/testfile"
export ZOIDFS_ION_NAME=tcp://127.0.0.1:1234

itr=1
for j in 1 2 4 8 16
do
    sum=0.0
    sumsq=0.0
    for i in $(seq 1 $itr)
    do
        val=`mpiexec -n 4 $IOFSL_DIR/test/zoidfs-microbenchmarks/zoidfs-tile-io --nr_tiles_x 1 --nr_tiles_y 4 --sz_tile_x $j --sz_tile_y $j --sz_element 262144 --write_file --cleanup --filename $OUTFILE | grep "Write Band" | awk '{print $4}'`
        sum=`echo "$sum + $val" | bc -l`
    done
    avg=`echo "$sum / $itr" | bc -l`
    echo "$j 1 4 $avg"
done
