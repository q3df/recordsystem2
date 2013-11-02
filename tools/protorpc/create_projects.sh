#!/bin/bash

rm -rf zz_tmp_build
mkdir zz_tmp_build
cd zz_tmp_build
cmake .. -G "Unix Makefiles" -DCMAKE_BUILD_TYPE="release" -DCMAKE_INSTALL_PREFIX=".." -DCMAKE_CXX_FLAGS_RELEASE="-std=c++0x -pthread -m32 -L/usr/lib32" -DCMAKE_EXE_LINKER_FLAGS="-std=c++0x -pthread -m32 -L/usr/lib32"
cd ..
