@echo off
REM Change to the directory that the .bat file resides in
cd /d %0\..
cmd.exe /c python.exe update-build-revision.py