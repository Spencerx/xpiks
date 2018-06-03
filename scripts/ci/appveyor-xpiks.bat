@echo off

set errorlevel=1
set errorlevel=

pushd src\xpiks-qt\
qmake "CONFIG+=%configuration% appveyor" xpiks-qt.pro
nmake.exe

if errorlevel 1 exit /b %errorlevel%

echo "Build finished for branch %APPVEYOR_REPO_BRANCH%"

popd

if "%APPVEYOR_REPO_BRANCH%" == "master" (
    cd scripts\deploy
    deploy_win.bat
    appveyor PushArtifact Xpiks-v%APPVEYOR_BUILD_VERSION%.zip
)
