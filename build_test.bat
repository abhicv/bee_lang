@echo off
@REM call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat"
if not exist bin mkdir bin
pushd bin
if exist lexer_test.exe del lexer_test.exe
cl /nologo /Zi ..\test\lexer_test.c /link /incremental:no /subsystem:console
if exist lexer_test.exe lexer_test.exe
popd bin


