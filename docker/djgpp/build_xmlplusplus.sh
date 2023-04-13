#!/bin/bash

export PKG_CONFIG_PATH=/usr/local/lib/pkgconfig

NOCONFIGURE=1 ./autogen.sh

CPPFLAGS="-I/usr/local/include/libxml2" ./configure \
--host=i586-pc-msdosdjgpp \
--enable-static=yes \
--enable-shared=no \
--disable-documentation

make -j`nproc`

make install
