@echo off
@REM call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat"
if not exist bin mkdir bin
pushd bin
cl /nologo /Zi ..\source\main.c /Fe:bee /link /incremental:no /subsystem:console
popd bin
