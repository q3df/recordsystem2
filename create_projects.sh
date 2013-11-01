#!/bin/sh
tools/gyp/gyp --depth "$(pwd)" -I recordsystem.gypi

cd tools/protorpc/
sh ./create_projects.sh
cd ../../
