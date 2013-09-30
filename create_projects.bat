@echo off
PATH=%PATH%;%CD%\tools\python_26\
tools\gyp\gyp.bat --depth "%CD%" -I recordsystem.gypi

