@echo off
set GYP_MSVS_VERSION=2010
PATH=%PATH%;%CD%\tools\python_26\
xcopy /Y /F .\rbnf.cpp.patched tools\icu\source\i18n\rbnf.cpp
IF EXIST tools\boost\include goto :FINISH
mkdir tools\boost\include
set OLDCD=%CD%
cd tools\boost\
..\7z\7z.exe x boost_win_headers.7z 2> NUL > NUL
move boost include\
cd %OLDCD%

:FINISH


@powershell -NoProfile -ExecutionPolicy Bypass -Command "$text = [IO.File]::ReadAllText(\"compileprotoc.sh\") -replace  \"`r`n\", \"`n\"; [IO.File]::WriteAllText(\"compileprotoc.sh\", $text)"
tools\gyp\gyp.bat --depth "%CD%" -I recordsystem.gypi
