#!/bin/bash

if [ -z "$1" ]; then
  echo "ERROR: Please specify a build target: Debug or Release"
else
  if [ -z "$2" ]; then
    PROJECT_NAME='qagamex86'
  else
    PROJECT_NAME=$2
  fi
  if [ `uname` = "Linux" ]; then
    make -j16 $PROJECT_NAME BUILDTYPE=$1
  else
    xcodebuild -project recordsystem.xcodeproj -configuration $1 -target "$PROJECT_NAME"
  fi
fi
