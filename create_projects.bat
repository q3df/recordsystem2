@echo off
set GYP_MSVS_VERSION=2010
PATH=%PATH%;%CD%\tools\python_26\
xcopy /Y /F .\rbnf.cpp.patched tools\icu\source\i18n\rbnf.cpp
tools\gyp\gyp.bat --depth "%CD%" -I recordsystem.gypi
