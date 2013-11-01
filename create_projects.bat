@echo off
PATH=%PATH%;%CD%\tools\python_26\
call tools\gyp\gyp.bat --depth "%CD%" -I recordsystem.gypi

cd tools\protorpc
call create_projects.bat
cd ..\..\
