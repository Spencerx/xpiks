@echo off

setlocal enabledelayedexpansion

set mode=%1
set errorlevel=1
set errorlevel=

if "%mode%" == "build" (
    pushd src\xpiks-tests\plugins-for-tests\helloworld\xpiks-helloworld-plugin
    qmake "CONFIG+=%configuration% appveyor" xpiks-helloworld-plugin.pro
    nmake.exe || exit /b !errorlevel!
    popd

    pushd src\xpiks-tests\xpiks-tests-integration
    qmake "CONFIG+=%configuration% appveyor" xpiks-tests-integration.pro
    nmake.exe || exit /b !errorlevel!
    popd
)

set testsexitcode=0

if "%mode%" == "run" (
    pushd src\xpiks-tests\xpiks-tests-integration
    copy ..\..\..\libs\%configuration%\*.dll .
    %configuration%\xpiks-tests-integration.exe > tests.log
	if errorlevel 1 (
		set testsexitcode=!errorlevel!
		echo Integration tests failed with code !testsexitcode!
		type tests.log
		goto :error
	)

    echo "Starting integration tests in memory..."
    %configuration%\xpiks-tests-integration.exe --in-memory  > tests_in_memory.log
    if errorlevel 1 (
        set testsexitcode=!errorlevel!
        echo In-memory tests failed with code !testsexitcode!
        type tests_in_memory.log
        goto :error
    )

    type tests.log
    popd
)

goto :EOF

:error
echo Failed with error #!testsexitcode!.
exit /b !testsexitcode!


