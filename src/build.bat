mkdir ..\build
pushd ..\build

REM SET CL=/DPERF_DIAGNOSTIC

cl /Zi /Od "..\src\sdl_platform.cpp" /Fe"game.exe" /I "D:\edvin\SDL2-2.0.22\include" /link /LIBPATH:D:\edvin\SDL2-2.0.22\lib\x64 user32.lib SDL2main.lib SDL2.lib shell32.lib /SUBSYSTEM:CONSOLE

SET CL=

popd
