@echo off
@REM call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat"
if not exist bin mkdir bin
pushd bin
if exist stack_vm_test.exe del stack_vm_test.exe
cl /nologo /Zi ..\test\stack_vm_test.c /link /incremental:no /subsystem:console
if exist stack_vm_test.exe stack_vm_test.exe
popd bin