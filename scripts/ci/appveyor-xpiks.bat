@echo off

set errorlevel=1
set errorlevel=
set CL=/MP

pushd src\xpiks-qt\
qmake "CONFIG+=%configuration% appveyor" xpiks-qt.pro
nmake.exe

if errorlevel 1 exit /b %errorlevel%

echo "Build finished for branch %APPVEYOR_REPO_BRANCH%"

popd

echo %cd%

set "shoulddeploy="
if "%APPVEYOR_REPO_BRANCH%" == "master" set shoulddeploy=1
if "%APPVEYOR_REPO_BRANCH%" == "nightly" set shoulddeploy=1

if defined shoulddeploy (
    pushd scripts\deploy
    deploy_win.bat
    appveyor PushArtifact Xpiks-v%APPVEYOR_BUILD_VERSION%.zip
    popd
)
