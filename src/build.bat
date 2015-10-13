@echo off

set SDL_INCLUDE="C:\Users\jabel\Documents\GitHub\SDL2-2.0.3\include"
set SDL_LIB="C:\Users\jabel\Documents\GitHub\SDL2-2.0.3\lib\x64"

set CommonCompilerFlags=-I%SDL_INCLUDE% -MDd -nologo -fp:fast -Gm- -GR- -EHa- -Od -Oi -WX -W4 -wd4456 -wd4201 -wd4100 -wd4189 -wd4505 -DBUILD_INTERNAL=1 -DBUILD_SLOW=1 -DBUILD_WIN32=1 -D_CRT_SECURE_NO_WARNINGS -FC -Z7
set CommonLinkerFlags=/LIBPATH:%SDL_LIB% -incremental:no -opt:ref user32.lib SDL2.lib SDL2main.lib  /SUBSYSTEM:WINDOWS /NODEFAULTLIB:msvcrt.lib

IF NOT EXIST ..\build mkdir ..\build
pushd ..\build

cl %CommonCompilerFlags% ..\src\main.cpp -FeTicTacToe.exe -FmTicTacToe.map /link %CommonLinkerFlags% 
popd
