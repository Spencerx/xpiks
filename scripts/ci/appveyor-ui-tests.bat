@echo off

setlocal enabledelayedexpansion

set errorlevel=1
set errorlevel=
set mode=%1

if "%mode%" == "build" (
    pushd src\xpiks-tests\ui-tests-stubs
    qmake "CONFIG+=%configuration% appveyor" ui-tests-stubs.pro
    nmake.exe || goto :error
    popd

    pushd src\xpiks-tests\xpiks-tests-ui
    qmake "CONFIG+=%configuration% appveyor" xpiks-tests-ui.pro
    nmake.exe || goto :error
    popd
)

if "%mode%" == "run" (
    pushd src\xpiks-tests\xpiks-tests-ui
    %configuration%\xpiks-tests-ui.exe || goto :error
    popd
)

goto :EOF

:error
echo Failed with error #!errorlevel!.
exit /b !errorlevel!