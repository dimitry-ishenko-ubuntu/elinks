#!/bin/bash
#
# shell script menus for elinks binaries building
#

clear

echo '       --/                                     \--'
echo '       --[ Welcome to the elinks build helper  ]--'
echo '       --[                                     ]--'
echo '       --[ [*] use option 1 to change arch     ]--'
echo '       --[ [*] use option 4 to config and make ]--'
echo '       --[ [*] use option 5 for config         ]--'
echo '       --[ [*] use option 6 for make           ]--'
echo '       --[ [*] use option 7 for test           ]--'
echo '       --[ [*] use option 8 for publishing     ]--'
echo '       --\                                     /--'
echo '                                                 '


JS_ENABLE=0

gen_conf() {
  ./autogen.sh
}

configure() {
  echo "--[ Configure starts in 2 seconds ]--"
  echo "--[ Compiler: " $1 "]--"
  echo "--[ Host    : " $2 "]--"
  sleep 2
  rm -f config.cache
  # Thanks rkd77 for discovery of jemmaloc needed
  # to correct openssl functionality
  # LIBS="-ljemalloc -lpthread -lm"  \
  # Update: Thanks to JF for this solution for solving
  # crashes using pthread 
  # -Wl,--whole-archive -lpthread -Wl,--no-whole-archive
  BUILD_CMD="time \
  CC='$1' \
  LD='$2' \
  LDFLAGS='$4' \
  CXX=$CXX_CUST \
  CFLAGS='$6' \
  LIBS='$5' \
  CXXFLAGS='$6' \
  PKG_CONFIG='./pkg-config.sh' \
  ./configure -C \
  --host='$3' \
  --prefix=/usr \
  --enable-256-colors \
  --enable-fastmem \
  --enable-utf-8 \
  --with-static \
  --with-openssl \
  --without-gpm \
  --without-quickjs \
  --disable-88-colors \
  --disable-backtrace \
  --disable-bittorrent \
  --disable-debug \
  --disable-cgi \
  --disable-combining \
  --disable-gopher \
  --disable-nls \
  --disable-ipv6 \
  --disable-sm-scripting \
  --disable-true-color \
  --without-bzlib \
  --without-brotli \
  --without-gnutls \
  --without-libev \
  --without-libevent \
  --without-lzma \
  --without-perl \
  --without-python \
  --without-ruby \
  --without-terminfo \
  --without-zlib \
  --without-zstd \
  --without-x"
  if [ $JS_ENABLE == 1 ]; then
    BUILD_CMD="${BUILD_CMD/without-quickjs/with-quickjs}"
  fi
  echo "$BUILD_CMD"
  bash -c "$BUILD_CMD"
  if [ $? -eq 0 ]; then
    echo "--[ Configuration Sucessfull ]--"
    # turn off warnings
    #sed -i 's/-Wall/-w/g' Makefile.config
    #sed -i 's/-lpthread/-pthread/g' Makefile.config
    #build
    return 0
  else
    echo "--[ Listing errors in config.log ]--"
    cat config.log | grep error | tail
    echo "--[ Configuration failed... ]--"
    return 1
  fi
}

# BUILD FUNCTION
build() {
  echo "--[ Build starts in 2 seconds ]--"
  sleep 2
  time make # --trace
  if [ $? -eq 0 ]; then
    echo "--[ ................ ]--"
    echo "--[ Build Sucessfull ]--"
    echo "--[ All Done.        ]--"
    echo "--[ ................ ]--"
    return 0
  else
    echo "--[ Build failed... ]--"
    return 1
  fi
}

test() {
  #
  # very basic to test binary
  # won't core dump
  #
  # for arm32: qemu-arm-static
  # for win64: wineconsole
  #
  #./src/elinks$1 \
  #--no-connect \
  #--dump \
  #./test/hello.html
  # more complete testing
  ./test.sh "$BIN_SUFFIX" "$ARCHIT"
}

pub() {
  ls -l ./src/elinks$1
  file ./src/elinks$1
  if [ ! -d ../bin ]; then
    mkdir ../bin
  fi
  cp ./src/elinks$1 ../bin/elinks_$2$1
  echo "--[ File Published to ../bin ]--"
}

info() {
  echo "--[ binary info ]--"
  if [ ! -f ../src/elinks$1 ]; then
    file ./src/elinks$1
    ls -lh ./src/elinks$1
    ls -l ./src/elinks$1
    if [ "$ARCHIT" = "win64" ] || [ "$ARCHIT" = "win32" ]; then
      wineconsole --backend=ncurses ./src/elinks$1 --version
    else
      ./src/elinks$1 --version
    fi
  else
    echo "--[*] No binary compiled."
  fi
}

set_arch() {
  if [ "$1" = "lin32" ]; then
    ARCHIT="$1"
    CC="i686-linux-gnu-gcc"
    LD="i686-linux-gnu-ld"
    MAKE_HOST="i686-linux-gnu"
    BIN_SUFFIX=""
    CXXFLAGS=""
    LDFLAGS=""
    LIBS=""
  elif [ "$1" = "lin64" ]; then
    ARCHIT="$1"
    CC="x86_64-linux-gnu-gcc"
    LD="x86_64-linux-gnu-ld"
    MAKE_HOST="x86_64-linux-gnu"
    BIN_SUFFIX=""
    CXXFLAGS=""
    LDFLAGS=""
    LIBS="-Wl,--whole-archive -lpthread -Wl,--no-whole-archive"
  elif [ "$1" = "win32" ]; then
    ARCHIT="$1"
    CC="i686-w64-mingw32-gcc"
    LD="i686-w64-mingw32-ld"
    MAKE_HOST="x86_64-w32-mingw32"
    BIN_SUFFIX=".exe"
    CXXFLAGS="-I/usr/local/include"
    LDFLAGS="-L/usr/local/lib"
    LIBS="-lws2_32 -Wl,--whole-archive -lpthread -Wl,--no-whole-archive"
    CXX_CUST="i686-w64-mingw32-g++"
  elif [ "$1" = "win64" ]; then
    ARCHIT="$1"
    CC="x86_64-w64-mingw32-gcc"
    LD="x86_64-w64-mingw32-ld"
    MAKE_HOST="x86_64-w64-mingw32"
    BIN_SUFFIX=".exe"
    CXXFLAGS="-I/usr/local/include"
    LDFLAGS="-L/usr/local/lib"
    LIBS="-lws2_32 -Wl,--whole-archive -lpthread -Wl,--no-whole-archive"
  elif [ "$1" = "djgpp" ]; then
    ARCHIT="$1"
    CC="i586-pc-msdosdjgpp-gcc"
    LD="i586-pc-msdosdjgpp-ld --allow-multiple-definition"
    MAKE_HOST="i586-pc-msdosdjgpp"
    BIN_SUFFIX=".exe"
    CXXFLAGS="-I/usr/local/include -I/home/elinks/.dosemu/drive_c/LINKS/watt32/inc"
    LDFLAGS="-L/usr/local/lib -L/home/elinks/.dosemu/drive_c/LINKS/watt32/lib"
    LIBS="-lwatt"
  elif [ "$1" = "arm32" ]; then
    ARCHIT="$1"
    CC="arm-linux-gnueabihf-gcc"
    LD="arm-linux-gnueabihf-ld"
    MAKE_HOST="arm-linux-gnu"
    BIN_SUFFIX=""
    CXXFLAGS=""
    LDFLAGS=""
    LDFLAGS="-L/usr/local/lib"
    LIBS="-Wl,--whole-archive -lpthread -Wl,--no-whole-archive"
  elif [ "$1" = "arm64" ]; then
    ARCHIT="$1"
    CC="aarch64-linux-gnu-gcc"
    LD="aarch64-linux-gnu-ld"
    MAKE_HOST="aarch64-linux-gnu"
    BIN_SUFFIX=""
    CXXFLAGS="-I/usr/local/include"
    LDFLAGS="-L/usr/local/lib"
    LIBS="-Wl,--whole-archive -lpthread -Wl,--no-whole-archive"
  elif [ "$1" = "native" ]; then
    ARCHIT="$1"
    CC="gcc"
    LD="ld"
    MAKE_HOST=""
    BIN_SUFFIX=""
    CXXFLAGS="-I/usr/local/include"
    LDFLAGS="-L/usr/local/lib"
    LIBS="-Wl,--whole-archive -lpthread -Wl,--no-whole-archive"
  fi
}

# ARCH SELECTION MENU
arch_menu() {
  MENU_ARCHS="$ARCHS null null null return"
  echo "[=] Build architecture selection menu"
  select SEL in $MENU_ARCHS; do
    echo "[=] Build architecture selection menu"
    if [ "$SEL" = "lin32" ]; then
      set_arch "$SEL"
    elif [ "$SEL" = "lin64" ]; then
      set_arch "$SEL"
    elif [ "$SEL" = "win64" ]; then
      set_arch "$SEL"
    elif [ "$SEL" = "win32" ]; then
      set_arch "$SEL"
    elif [ "$SEL" = "arm32" ]; then
      set_arch "$SEL"
    elif [ "$SEL" = "arm64" ]; then
      set_arch "$SEL"
    elif [ "$SEL" = "djgpp" ]; then
      set_arch "$SEL"
    elif [ "$SEL" = "native" ]; then
      set_arch native
    elif [ "$SEL" = "make" ]; then
      build
    elif [ "$SEL" = "null" ]; then
      echo "[.] This option is intentially left blank"
    elif [ "$SEL" = "return" ]; then
      break
    fi
    echo "--[ Architecture : " $ARCHIT " ]--"
    echo "--[ Compiler     : " $CC " ]--"
    echo "--[ Host         : " $MAKE_HOST " ]--"
  done
}

# MAIN LOOP
CMDACT=$1
ARCHIT=""
BIN_SUFFIX=""
ARCHS="lin32 lin64 win32 win64 arm32 arm64 djgpp native"
CC_SEL="arch null null build \
config make test \
pub debug \
info \
build_all \
exit"
set_arch native
# command line action
if [ ! -z $CMDACT ]; then
  if [ $CMDACT == "build" ]; then
    JS_ENABLE=1
    CC=g++
    ./autogen.sh
    configure "$CC" "$LD" "$MAKE_HOST" "$LDFLAGS" "$LIBS" "$CXXFLAGS"
    if [ $? -eq 1 ]; then
      exit
    fi
    sed -i 's/^LIBS = .*/LIBS = -ltre -lssl -lcrypto -ldl -Wl,--whole-archive -lpthread -Wl,--no-whole-archive -lidn -lexpat  \/usr\/local\/lib\/quickjs\/libquickjs.a \/usr\/local\/lib\/libxml++-5.0.a -lxml2 -lz -licui18n -llzma -lsqlite3 -licuuc -licudata/g' Makefile.config
    build
    if [ $? -eq 1 ]; then
      exit
    fi
    exit
  fi
fi
# user mode action
select SEL in $CC_SEL; do
  if [ "$SEL" = "arch" ]; then
    set_arch native
    arch_menu
  elif [ "$SEL" = "build" ]; then
    configure "$CC" "$LD" "$MAKE_HOST" "$LDFLAGS" "$LIBS" "$CXXFLAGS"
    if [ $? -eq 1 ]; then
      break
    fi
    build
    if [ $? -eq 1 ]; then
      break
    fi
  elif [ "$SEL" = "make" ]; then
    build
  elif [ "$SEL" = "config" ]; then
    configure "$CC" "$LD" "$MAKE_HOST" "$LDFLAGS" "$LIBS" "$CXXFLAGS"
  elif [ "$SEL" = "test" ]; then
    test $BIN_SUFFIX
  elif [ "$SEL" = "pub" ]; then
    pub "$BIN_SUFFIX" "$ARCHIT"
  elif [ "$SEL" = "debug" ]; then
    gdb ./src/elinks$BIN_SUFFIX
  elif [ "$SEL" = "info" ]; then
    info "$BIN_SUFFIX"
  elif [ "$SEL" = "build_all" ]; then
    #set -f  # avoid globbing (expansion of *).
    arch_arr=($ARCHS)
    for arch in "${arch_arr[@]}"; do
      echo "--[ Building: $arch ]--"
      set_arch "$arch"
      configure "$CC" "$LD" "$MAKE_HOST" "$LDFLAGS" "$LIBS" "$CXXFLAGS"
      if [ $? -eq 1 ]; then
        break
      fi
      build
      if [ $? -eq 1 ]; then
        break
      fi
      info "$BIN_SUFFIX"
      pub "$BIN_SUFFIX" "$ARCHIT"
    done
  elif [ "$SEL" = "null" ]; then
    echo "[.] This option is intentially left blank"
  elif [ "$SEL" = "exit" ]; then
    exit
  fi
  echo "--[ [=] elinks build system main menu ]--"
  echo "--[ Architecture : " $ARCHIT " ]--"
  echo "--[ Compiler     : " $CC " ]--"
  echo "--[ Host         : " $MAKE_HOST " ]--"
done
