@echo off

cls

call "comp-shaders.bat"

cmake -S . -B build
cmake --build build

"./build/main.exe"

if not errorlevel 0 echo Exit code: %ERRORLEVEL%