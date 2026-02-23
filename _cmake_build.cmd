@echo off
call "C:\Program Files\Microsoft Visual Studio\18\Community\Common7\Tools\VsDevCmd.bat" -arch=x64
cmake --build out\build\nmake-x64 --target Orphan --config Release -j 8
