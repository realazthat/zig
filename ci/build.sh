

#be verbose, exit on error
set -exv


PROJECT_PATH="$PWD"

mkdir -p ./build && cd ./build

ZIG_LIBC_LIB_DIR=$(dirname $(cc -print-file-name=crt1.o))
ZIG_LIBC_INCLUDE_DIR=/usr/include
ZIG_LIBC_STATIC_LIB_DIR=$(dirname $(cc -print-file-name=crtbegin.o))

cmake .. \
    -DCMAKE_VERBOSE_MAKEFILE=1 \
    -DLLVM_CONFIG_EXE=$LLVM_CONFIG \
    -DCMAKE_BUILD_TYPE="$CMAKE_BUILD_TYPE" \
    -DCMAKE_INSTALL_PREFIX="${PWD}" \
    -DZIG_LIBC_LIB_DIR="$ZIG_LIBC_LIB_DIR" \
    -DZIG_LIBC_INCLUDE_DIR="$ZIG_LIBC_INCLUDE_DIR" \

cmake --build .

cmake --build . --target install


