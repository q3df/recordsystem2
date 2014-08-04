#!/bin/bash

if [ ! -d "mysql-connector-c-6.1.5-src" ]; then
	unzip mysql-connector-c-6.1.5-src.zip
fi

if [ ! -d "mysql-connector-c++-1.1.3" ]; then
	tar -xzf mysql-connector-c++-1.1.3.tar.gz
fi

cd mysql-connector-c-6.1.5-src
cmake -D CMAKE_INSTALL_PREFIX="../build/mysql/" -D PKG_INSTALL_PREFIX="../build/mysql/" -D CMAKE_C_FLAGS="-m32 -Wall -Wextra -Wformat-security -Wvla -Wwrite-strings -Wdeclaration-after-statement" -D CMAKE_CXX_FLAGS="-m32 -Wall -Wextra -Wformat-security -Wvla -Woverloaded-virtual -Wno-unused-parameter" .
make
make install

cd ../mysql-connector-c++-1.1.3
cmake -D CMAKE_INSTALL_PREFIX="../build/mysql/" -D CMAKE_CXX_FLAGS="-m32 -I../mysql-connector-c-6.1.5-src/include -DBIG_JOINS=1 -fno-strict-aliasing -g" .
make
make install

