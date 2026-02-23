@echo off
call "C:\Program Files\Microsoft Visual Studio\18\Community\Common7\Tools\VsDevCmd.bat" -arch=x64
cmake -S . -B out\build\nmake-x64 -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=Release -DORPHAN_PRIVATE_BUILD=ON -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -Wno-dev
