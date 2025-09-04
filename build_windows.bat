@echo off
if not exist bin mkdir bin
pushd bin
@REM cl /EHsc /c ..\source\instrument.c
@REM cl /nologo /EHsc /Gh /GH /Zi ..\source\main.c instrument.obj /Fo:bee /Fe:bee /link /incremental:no /subsystem:console
cl /nologo /Zi ..\source\main.c /Fo:bee /Fe:bee /link /map:bee.map /profile /incremental:no /subsystem:console
@REM cl /nologo /Zi ..\source\stack_vm.c /Fo:bee /Fe:stack_vm /link /map:bee.map /profile /incremental:no /subsystem:console
popd bin
