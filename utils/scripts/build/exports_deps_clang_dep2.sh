#!/usr/bin/env bash

export CC=clang 
export CXX=clang++

export LLVMCONFIG=$(which llvm-config)

export LLVM_DIR=$(${LLVMCONFIG} --prefix)/share/llvm/cmake/

export GTEST_ROOT=/usr/local/gtest-libcxx

export BOOST_ROOT=/bulk/workbench/boost/015900/install/

export CXX_FLAGS="-stdlib=libc++"

export LINKER_FLAGS="-Wl,-L$(${LLVMCONFIG} --libdir)" 
export LINKER_FLAGS="${LINKER_FLAGS} -lc++ -lc++abi" 

export BUILD_TYPE=Debug

export PRJ_USE_LLVM_INTERNAL_MODULE=OFF
export LOOPRUNTIMEPROFILER_SKIP_TESTS=OFF
export LOOPRUNTIMEPROFILER_DEBUG=ON

[[ -z ${AnnotateLoops_DIR} ]] && echo "missing AnnotateLoops_DIR"
[[ -z ${SimplifyLoopExits_DIR} ]] && echo "missing SimplifyLoopExits_DIR"
export AnnotateLoops_DIR
export SimplifyLoopExits_DIR


CMAKE_OPTIONS="-DLLVM_DIR=${LLVM_DIR}"
CMAKE_OPTIONS="${CMAKE_OPTIONS} -DBOOST_ROOT=${BOOST_ROOT}"
CMAKE_OPTIONS="${CMAKE_OPTIONS} -DGTEST_ROOT=${GTEST_ROOT}"
CMAKE_OPTIONS="${CMAKE_OPTIONS} -DPRJ_USE_LLVM_INTERNAL_MODULE=${PRJ_USE_LLVM_INTERNAL_MODULE}"
CMAKE_OPTIONS="${CMAKE_OPTIONS} -DLOOPRUNTIMEPROFILER_DEBUG=${LOOPRUNTIMEPROFILER_DEBUG}"
CMAKE_OPTIONS="${CMAKE_OPTIONS} -DLOOPRUNTIMEPROFILER_SKIP_TESTS=${LOOPRUNTIMEPROFILER_SKIP_TESTS}"
CMAKE_OPTIONS="${CMAKE_OPTIONS} -DAnnotateLoops_DIR=${AnnotateLoops_DIR}"
CMAKE_OPTIONS="${CMAKE_OPTIONS} -DSimplifyLoopExits_DIR=${SimplifyLoopExits_DIR}"

export CMAKE_OPTIONS

