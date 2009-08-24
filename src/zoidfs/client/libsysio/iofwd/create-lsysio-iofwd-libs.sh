#!/bin/bash

mkdir ./libbuild
cd ./libbuild

ar -x /home/copej/mac-copej/work/anl/mcs/iofwd-64bit/bmi-2.8.1/lib/libbmi.a
ar -x /home/copej/mac-copej/work/anl/mcs/iofwd-64bit/iofwd/src/zoidfs/libzoidfsclient.a
#ar -x /home/copej/mac-copej/work/anl/mcs/iofwd-64bit/iofwd/src/zoidfs/libzoidfsdispatch.a
ar -x /home/copej/mac-copej/work/anl/mcs/iofwd-64bit/libsysio-iofwd-update/lib/libsysio.a
#ar -x /home/copej/mac-copej/work/anl/mcs/iofwd-64bit/libsysio/lib/libsysio.a

# static lib
ar -cru ./libiofwdsysio.a ./*.o
mv ./libiofwdsysio.a ../

# shared lib
#gcc -shared -Wl,-soname,libiofwdsysio.so.1 -o libsysioiofwd.so.1.0 *.o

rm -f ./*.o
rm -f ./*.a
cd ..

rmdir ./libbuild

# make the client app
#gcc -c test.c -o test.o
gcc -DIOFSL_ALT_SYMBOL=_iofwd -c test.c -o test.o
gcc test.o -o test ./libiofwdsysio.a -lpthread
