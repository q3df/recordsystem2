@echo off
set GYP_MSVS_VERSION=2010
PATH=%PATH%;%CD%\tools\python_26\
tools\gyp\gyp.bat --depth "%CD%" -I recordsystem.gypi
