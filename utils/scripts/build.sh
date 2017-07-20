#!/usr/bin/env bash

# initialize configuration vars

SRC_DIR=""
INSTALL_DIR=""


# set configuration vars

if [ -z "$1" ]; then 
  echo "error: source directory was not provided" 

  exit 1
fi

SRC_DIR=$1

if [ -z "$2" ]; then 
  INSTALL_DIR="${SRC_DIR}/../install/"
else
  INSTALL_DIR="$2"
fi


# print configuration vars

echo "info: printing configuration vars"
echo "info: source dir: ${SRC_DIR}"
echo "info: install dir: ${INSTALL_DIR}"
echo ""


LINKER_FLAGS="-Wl,-L$(llvm-config --libdir) -lc++ -lc++abi" 

CC=clang CXX=clang++ \
  cmake \
  -DCMAKE_EXPORT_COMPILE_COMMANDS=On \
  -DLLVM_DIR=$(llvm-config --prefix)/share/llvm/cmake/ \
  -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_CXX_FLAGS="-stdlib=libc++" \
  -DCMAKE_EXE_LINKER_FLAGS="${LINKER_FLAGS}" \
  -DCMAKE_SHARED_LINKER_FLAGS="${LINKER_FLAGS}" \
  -DCMAKE_MODULE_LINKER_FLAGS="${LINKER_FLAGS}" \
  -DCMAKE_INSTALL_PREFIX="${INSTALL_DIR}" \
  -DPRJ_USE_LLVM_INTERNAL_MODULE=OFF \
  -DPRJ_SKIP_TESTS=OFF \
  -DPRJ_DEBUG=ON \
  -DBOOST_ROOT=${BOOST_ROOT} \
  -DGTEST_ROOT=${GTEST_ROOT} \
  "${SRC_DIR}"


exit $?

