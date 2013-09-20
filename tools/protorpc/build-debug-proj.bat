@rem Copyright 2013 <chaishushan{AT}gmail.com>. All rights reserved.
@rem Use of this source code is governed by a BSD-style
@rem license that can be found in the LICENSE file.

@rem --------------------------------------------------------------------------
@rem build app

mkdir zz_build_debug_proj_tmp
cd    zz_build_debug_proj_tmp

cmake ..^
  -DCMAKE_BUILD_TYPE=debug^
  -DCMAKE_INSTALL_PREFIX=..^
  ^
  -DCMAKE_C_FLAGS_DEBUG="/MDd /Zi /Od /Ob0 /RTC1"^
  -DCMAKE_CXX_FLAGS_DEBUG="/MDd /Zi /Od /Ob0 /RTC1"^
  ^
  -DCMAKE_C_FLAGS_RELEASE="/MD /O2 /Ob2 /DNDEBUG"^
  -DCMAKE_CXX_FLAGS_RELEASE="/MD /O2 /Ob2 /DNDEBUG"^
  ^
  -DCMAKE_EXE_LINKER_FLAGS="/MANIFEST:NO"
  
@rem --------------------------------------------------------------------------
cd ..
