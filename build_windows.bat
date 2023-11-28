@echo off
if not exist bin mkdir bin
pushd bin
cl /nologo /O2 ..\source\main.c /Fo:bee /Fe:bee /link /incremental:no /subsystem:console
popd bin
