@echo off

setlocal enabledelayedexpansion

set errorlevel=1
set errorlevel=
set mode=%1

if "%mode%" == "build" (
    echo "Building UI tests..."

    pushd src\xpiks-tests\xpiks-tests-ui
    qmake "CONFIG+=%configuration% appveyor" xpiks-tests-ui.pro
    nmake.exe || goto :error
    popd
)

set testsexitcode=0

if "%mode%" == "run" (
    pushd src\xpiks-tests\xpiks-tests-ui
    copy ..\..\..\libs\%configuration%\*.dll .

    echo "Running UI tests..."
    %configuration%\xpiks-tests-ui.exe > uitests_in_memory.log
    if errorlevel 1 (
        set testsexitcode=!errorlevel!
        echo UI tests failed with code !testsexitcode!
        type uitests_in_memory.log
        goto :error
    )

    type uitests_in_memory.log
    appveyor PushArtifact uitests_in_memory.log
    popd
)

goto :EOF

:error
echo "Handling error..."
appveyor PushArtifact uitests_in_memory.log

appveyor PushArtifact %configuration%\xpiks-tests-ui.exe
appveyor PushArtifact %configuration%\xpiks-tests-ui.pdb

for %%f in (*.dmp) do (
    echo Trying to upload dump %%~nf
    appveyor PushArtifact %%~nf
)

popd
echo Failed with error #!testsexitcode!
exit /b !testsexitcode!

