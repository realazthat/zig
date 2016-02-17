

#be verbose, exit on error
set -exv


PROJECT_PATH="$PWD"

mkdir -p ./build && cd ./build

ZIG_LIBC_LIB_DIR=/usr/lib/x86_64-linux-gnu/ 
ZIG_LIBC_INCLUDE_DIR=/usr/include

cmake .. \
    -DCMAKE_VERBOSE_MAKEFILE=1 \
    -DCMAKE_BUILD_TYPE="$CMAKE_BUILD_TYPE" \
    -DCMAKE_INSTALL_PREFIX="${PWD}" \
    -DZIG_LIBC_LIB_DIR="$ZIG_LIBC_LIB_DIR" \
    -DZIG_LIBC_INCLUDE_DIR="$ZIG_LIBC_INCLUDE_DIR" \

cmake --build .

cmake --build . --target install


