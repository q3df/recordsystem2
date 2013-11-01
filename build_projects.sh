#!/bin/bash

if [ -z "$1" ]; then
  echo "ERROR: Please specify a build target: Debug or Release"
else
  cd tools/protorpc
  sh build.sh
  cd ../../

  if [ `uname` = "Linux" ]; then
    make -j16 all BUILDTYPE=$1
  else
    xcodebuild -project recordsystem.xcodeproj -configuration $1
  fi
fi
