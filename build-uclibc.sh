#!/bin/bash
cd uC*
make -j8
cp lib/libc.a ../libs/uclibc.a
make PREFIX=install-root install
cp -r install-root/usr/x86_64-linux-uclibc/usr/include/* ../uclibc-include
