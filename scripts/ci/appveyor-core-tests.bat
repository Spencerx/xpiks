@echo off

setlocal enabledelayedexpansion

set errorlevel=1
set errorlevel=
set mode=%1

pushd src\xpiks-tests\xpiks-tests-core

if "%mode%" == "build" (
    qmake "CONFIG+=%configuration% appveyor" xpiks-tests-core.pro
    nmake.exe || goto :error
)

if "%mode%" == "run" (
    copy ..\..\..\libs\%configuration%\*.dll .
    %configuration%\xpiks-tests-core.exe || goto :error
)

popd

goto :EOF

:error
echo Failed with error #!errorlevel!.
exit /b !errorlevel!