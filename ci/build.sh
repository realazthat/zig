

#be verbose, exit on error
set -exv


PROJECT_PATH="$PWD"

mkdir -p ./build && cd ./build

ZIG_LIBC_LIB_DIR=$(dirname $(cc -print-file-name=crt1.o))
ZIG_LIBC_INCLUDE_DIR=/usr/include
ZIG_LIBC_STATIC_LIB_DIR=$(dirname $(cc -print-file-name=crtbegin.o))
LLVM_BASE_LIBRARY=`$LLVM_CONFIG_EXE --libs`
LLVM_SYSTEM_LIBRARY=`$LLVM_CONFIG_EXE --system-libs`
LLVM_LIBRARY_DIR=`$LLVM_CONFIG_EXE --libdir`
LLVM_INCLUDE_DIR=`$LLVM_CONFIG_EXE --includedir`


cmake .. \
    -G"$CMAKE_GENERATOR" \
    -DCMAKE_VERBOSE_MAKEFILE=1 \
    -DCMAKE_BUILD_TYPE="$CMAKE_BUILD_TYPE" \
    -DCMAKE_INSTALL_PREFIX="${PWD}" \
    -DLLVM_CONFIG_EXE=`which $LLVM_CONFIG_EXE` \
    -DZIG_LIBC_LIB_DIR="$ZIG_LIBC_LIB_DIR" \
    -DZIG_LIBC_INCLUDE_DIR="$ZIG_LIBC_INCLUDE_DIR" \
    -DZIG_LIBC_STATIC_LIB_DIR="$ZIG_LIBC_STATIC_LIB_DIR"

cmake -LAH .

cmake --build .

cmake --build . --target install


