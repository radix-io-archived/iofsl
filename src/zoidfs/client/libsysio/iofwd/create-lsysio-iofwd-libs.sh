#!/bin/bash

#
# create-lsysio-iofwd-libs.sh 
#
# This script creates a libsysio / iofsl client library for posix applications.
#  The current version of this build script focuses on BMI TCP support.
#
# Created By: Jason Cope
# Email: copej@mcs.anl.gov
# Last Updated: 8/26/2009
#

# create the tmp build dir
mkdir ./libbuild
cd ./libbuild

# parse the BMI library
mkdir ./libbmi
cd ./libbmi
ar -x /home/copej/mac-copej/work/anl/mcs/iofwd-64bit/bmi-2.8.1/lib/libbmi.a
cd ..

# parse the zoidfsclient lib
mkdir ./libzoidfsclient
cd ./libzoidfsclient
ar -x /home/copej/mac-copej/work/anl/mcs/iofwd-64bit/tmp/iofwd/src/zoidfs/libzoidfsclient.a
ar -x /home/copej/mac-copej/work/anl/mcs/iofwd-64bit/tmp/iofwd/src/zoidfs/libzoidfshints.a
cd ..

# parse the libsysio
mkdir ./libsysio
cd ./libsysio
ar -x /home/copej/mac-copej/work/anl/mcs/iofwd-64bit/libsysio-iofwd-update/lib/libsysio.a
cd ..

# parse libc
#mkdir ./libc
#cd ./libc
#ar -x /usr/lib/libc.a close.o fcntl.o read.o write.o readv.o writev.o sysdep.o
#cd ..

# compile the syscall wrapper
gcc -g -fPIC -c ../iofsl_syscall_wrapper.c -o iofsl_syscall_wrapper.o
#mv ./iofsl_syscall_wrapper.o ../

# static lib build

# bmi-tcp.o and sockio.o

#
# use objcopy to redfine the syscall symbols to the wrappers
#
objcopy --redefine-sym fcntl=_iofwdlibsysio_wrap_fcntl --redefine-sym close=_iofwdlibsysio_wrap_close --redefine-sym read=_iofwdlibsysio_wrap_read --redefine-sym write=_iofwdlibsysio_wrap_write --redefine-sym readv=_iofwdlibsysio_wrap_readv --redefine-sym writev=_iofwdlibsysio_wrap_writev ./libbmi/bmi-tcp.o ./libbmi/bmi-tcp-ext.o
objcopy --redefine-sym fcntl=_iofwdlibsysio_wrap_fcntl --redefine-sym close=_iofwdlibsysio_wrap_close --redefine-sym read=_iofwdlibsysio_wrap_read --redefine-sym write=_iofwdlibsysio_wrap_write --redefine-sym readv=_iofwdlibsysio_wrap_readv --redefine-sym writev=_iofwdlibsysio_wrap_writev ./libbmi/sockio.o ./libbmi/sockio-ext.o

#
# Use ld to redefine symbols
#
#ld -r -x -defsym fcntl=_iofwdlibsysio_wrap_fcntl -defsym close=_iofwdlibsysio_wrap_close -defsym read=_iofwdlibsysio_wrap_read -defsym write=_iofwdlibsysio_wrap_write -defsym readv=_iofwdlibsysio_wrap_readv -defsym writev=_iofwdlibsysio_wrap_writev ../iofsl_syscall_wrapper.o ./libbmi/bmi-tcp.o ./libbmi/sockio.o -o ./libbmi/bmi-io-ext.o

#
# remove the old object files
#
rm ./libbmi/bmi-tcp.o ./libbmi/sockio.o

#
# create the libsysio / iofsl client library
#
ar -cru ./libiofwdsysio.a ./iofsl_syscall_wrapper.o ./libbmi/*.o ./libzoidfsclient/*.o ./libsysio/*.o
mv ./libiofwdsysio.a ../

#
# create the shared library
#
gcc -shared -Wl,-soname,libiofwdsysio.so.1 -o libiofwdsysio.so.1.0.1 ./iofsl_syscall_wrapper.o ./libbmi/*.o ./libzoidfsclient/*.o ./libsysio/*.o
mv ./libiofwdsysio.so.1.0.1 ../

# cleanup and go back to the base dir
rm -rf ./libbmi
rm -rf ./libzoidfsclient
rm -rf ./libsysio
cd ..
rm -rf ./libbuild

rm -rf ./sharedlibs
rm -rf ./staticlibs

mkdir ./sharedlibs
mkdir ./staticlibs

mv ./libiofwdsysio.so.1.0.1 ./sharedlibs
ln -s ./libiofwdsysio.so.1.0.1 ./sharedlibs/libiofwdsysio.so.1
ln -s ./libiofwdsysio.so.1 ./sharedlibs/libiofwdsysio.so
mv ./libiofwdsysio.a ./staticlibs

#
# make the client app (normal symbols)
#

gcc -g -c posix-cunit.c -o posix-cunit.o
gcc posix-cunit.o -g -o posix-cunit-static -L/home/copej/mac-copej/work/anl/mcs/iofwd-64bit/tmp/iofwd/src/zoidfs -lzoidfsclient ./staticlibs/libiofwdsysio.a -lpthread -lrt -L/home/copej/mac-copej/work/anl/mcs/iofwd-64bit/tmp/iofwd/src/zoidfs -lzoidfsclient -L/home/copej/mac-copej/work/anl/mcs/iofwd-64bit/tmp/iofwd/src/zoidfs -lzoidfshints
#gcc posix-cunit.o -g -o posix-cunit-shared -L./sharedlibs -liofwdsysio -lpthread

#
# make the test client app (alternate symbols)
#
#gcc -g -DIOFSL_ALT_SYMBOL=_zfs_sysio -c posix-cunit.c -o posix-cunit-native.o
#gcc -o posix-cunit-native posix-cunit-native.o ~/mac-copej/work/anl/mcs/iofwd-64bit/libsysio/lib/libsysio.a -lpthread
