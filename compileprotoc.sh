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

if [ -f "../tools/protorpc/bin/protoc.exe" ]; then
	../tools/protorpc/bin/protoc.exe --cxx_out=./ $2
fi

if [ -f "../tools/protorpc/bin/protoc" ]; then
	../tools/protorpc/bin/protoc --cxx_out=./ $2
fi

cd $OLDCD

