#!/bin/sh
cp ./rbnf.cpp.patched tools/icu/source/i18n/rbnf.cpp

if [ ! -d "tools/boost/include" ]; then
	cd tools/boost/
	mkdir include/
	7z x boost_linux_headers.7z
	mv boost include
	cd ../..
fi

tools/gyp/gyp --depth "$(pwd)" -I recordsystem.gypi
