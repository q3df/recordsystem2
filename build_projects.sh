#!/bin/bash

cp ./rbnf.cpp.patched tools/icu/source/i18n/rbnf.cpp

if [ -z "$1" ]; then
  echo "ERROR: Please specify a build target: Debug or Release"
else
  if [ `uname` = "Linux" ]; then
    make -j4 all BUILDTYPE=$1
  else
    xcodebuild -project recordsystem.xcodeproj -configuration $1
  fi
fi
