#!/bin/sh
cp ./rbnf.cpp.patched tools/icu/source/i18n/rbnf.cpp
tools/gyp/gyp --depth "$(pwd)" -I recordsystem.gypi
