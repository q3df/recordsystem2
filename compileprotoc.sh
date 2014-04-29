#!/bin/bash

if [ ! -d "$1" ]; then
	echo "$1 is not a directory"
	exit 1
fi

if [ ! -f "$1/$2" ]; then
	echo "$1/$2 file not found!"
	exit 1
fi

OLDCD=$(pwd)
cd $1

if [ -f "../../build/Release/protoc.exe" ]; then
	echo ../../build/Release/protoc.exe --cxx_out=./ $2
	../../build/Release/protoc.exe --cxx_out=./ $2
else
	if [ -f "../../build/Debug/protoc.exe" ]; then
		echo ../../build/Debug/protoc.exe --cxx_out=./ $2
		../../build/Debug/protoc.exe --cxx_out=./ $2
	fi
fi

if [ -f "../../out/Debug/protoc" ]; then
	../../out/Debug/protoc --cxx_out=./ $2
else
	if [ -f "../../out/Release/protoc" ]; then
		../../out/Release/protoc --cxx_out=./ $2
	fi
fi

cd $OLDCD

