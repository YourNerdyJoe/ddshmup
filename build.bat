@echo off

if not exist build mkdir build
REM
REM Paths relative to build not .
REM
pushd build
set targetname=game
set libname=lib%targetname%
set dllsourcefiles=..\src\*.cpp ..\src\pc\*.c
set exesourcefiles=..\src\pc\main\*.c ..\deps\lib\x64\SDL2main.lib
set libraryfiles=..\deps\lib\x64\SDL2.lib user32.lib gdi32.lib shell32.lib
set compilerflags=/std:c++latest /W3 /WX /permissive- /D_CRT_SECURE_NO_WARNINGS /GR- /Zi /MDd /Oi /Od /I..\deps\include /D_HAS_EXCEPTIONS=0 /nologo /MP
set linkerflags=/link /OPT:REF /SUBSYSTEM:CONSOLE

del ..\%libname%_*.pdb
echo on
cl %compilerflags% %dllsourcefiles% %libraryfiles% /Fe:..\%libname%.dll /Fm:%libname%.map %linkerflags% /DLL /PDB:..\%libname%_%random%.pdb
@IF "%1" == "dllonly" GOTO Done
cl %compilerflags% %exesourcefiles% %libraryfiles% /Fe:..\%targetname%.exe /Fm:%targetname%.map %linkerflags%
:Done
@popd
@del %libname%.exp %libname%.lib
