#!/bin/bash
#set -x
# minimal compile script.  don't use: use buildall instead. -ezk

CFLAGS="-I/usr/local/dataseries/include -I/usr/include/libxml2"

g++ $CFLAGS $LDFLAGS -c -o DataSeriesOutputModule.o DataSeriesOutputModule.cpp || exit $?
g++ $CFLAGS $LDFLAGS -c -o strace2ds.o strace2ds.cpp || exit $?
ar rv libstrace2ds.a strace2ds.o DataSeriesOutputModule.o || exit $?
cp -av libstrace2ds.a ~/strace2ds/lib || exit $?
