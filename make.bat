@echo off

cls

cmake -S . -B build
cmake --build build

"./build/main.exe"

if not errorlevel 0 echo Exit code: %ERRORLEVEL%