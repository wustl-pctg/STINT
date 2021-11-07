#!/bin/bash

NPROCS_TOUSE=$(nproc)

BINUTILS_INC=/usr/include

git clone -b neboat/cilksan https://github.com/OpenCilk/opencilk-project.git

# git checkout -b neboat/cilksan origin/neboat/cilksan

pushd .

cd opencilk-project/llvm

# cp -r ../clang ./tools
# cp -r ../compiler-rt/  projects

mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release -DLLVM_TARGETS_TO_BUILD=host -DLLVM_BINUTILS_INCDIR=$BINUTILS_INC -DLLVM_ENABLE_PROJECTS="clang;compiler-rt" -DLLVM_ENABLE_ASSERTIONS=On ..
cmake --build . -- -j$NPROCS_TOUSE

cd ../..

popd
