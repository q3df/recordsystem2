@echo off
set GYP_MSVS_VERSION=2010
PATH=%PATH%;%CD%\tools\python_26\
xcopy /Y /F .\rbnf.cpp.patched tools\icu\source\i18n\rbnf.cpp
IF EXIST tools\boost\include goto :FINISH
mkdir tools\boost\include
OLDCD=%CD%
cd tools\boost\
..\7z\7z.exe x boost_win_headers.7z
move boost include\
cd %OLDCD%

:FINISH
tools\gyp\gyp.bat --depth "%CD%" -I recordsystem.gypi
