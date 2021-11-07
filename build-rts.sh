#!/bin/sh

RTS_PATH=`pwd`/rts
LLVM_HOME=`pwd`/opencilk-project/llvm/build

cd cilkplus-rts
libtoolize
aclocal
automake --add-missing
autoconf
./configure --prefix=$RTS_PATH CC=$LLVM_HOME/bin/clang CXX=$LLVM_HOME/bin/clang++
make
make install
cd -
cp -r cilkplus-rts/include/internal $RTS_PATH/include
