rmdir /Q /S zz_build_tmp
mkdir zz_build_tmp
cd    zz_build_tmp

cmake ..^
  -G "NMake Makefiles"^
  -DCMAKE_BUILD_TYPE=release^
  -DCMAKE_INSTALL_PREFIX=..^
  ^
  -DCMAKE_C_FLAGS_DEBUG="/MDd /Zi /Od /Ob0 /RTC1"^
  -DCMAKE_CXX_FLAGS_DEBUG="/MDd /Zi /Od /Ob0 /RTC1"^
  ^
  -DCMAKE_C_FLAGS_RELEASE="/MD /Od /Zi /Ob2 /DNDEBUG"^
  -DCMAKE_CXX_FLAGS_RELEASE="/MD /Od /Zi /Ob2 /DNDEBUG"^
  ^
  -DCMAKE_EXE_LINKER_FLAGS="/MANIFEST:NO"
  
cd ..
