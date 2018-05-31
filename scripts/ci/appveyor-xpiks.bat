@echo off

set errorlevel=1
set errorlevel=

pushd src\xpiks-qt\
qmake "CONFIG+=%configuration% appveyor" xpiks-qt.pro
nmake.exe

if errorlevel 1 exit /b %errorlevel%

echo "Build finished for branch %APPVEYOR_REPO_BRANCH%"

if "%APPVEYOR_REPO_BRANCH%" == "master" (
    echo "Creating deployment package..."
    cd %configuration%
    windeployqt.exe --debug --qmldir=../../xpiks-qt/CollapserTabs/ --qmldir=../../xpiks-qt/Components/ --qmldir=../../xpiks-qt/Constants/ --qmldir=../../xpiks-qt/Dialogs/ --qmldir=../../xpiks-qt/StackViews/ --qmldir=../../xpiks-qt/StyledControls/ --qmldir=../../xpiks-qt/ Xpiks.exe
    xcopy /Y /s c:\projects\xpiks\src\xpiks-qt\deps .
    xcopy /Y /s c:\projects\xpiks-deps\windows-libs\ffmpeg-x64\debug\*.dll .
    copy /Y ..\..\..\libs\%configuration%\libcurl*.dll .
    copy /Y ..\..\..\libs\%configuration%\quazip*.dll .
    copy /Y ..\..\..\libs\%configuration%\ssdll.dll .
    copy /Y ..\..\..\libs\%configuration%\mman.dll .
    copy /Y ..\..\..\libs\%configuration%\face.dll .
    copy /Y ..\..\..\libs\%configuration%\z.dll .
    del *.obj
    del *.cpp
    cd ..
    7z a xpiks-qt-%configuration%.zip %configuration%
    appveyor PushArtifact xpiks-qt-%configuration%-%APPVEYOR_BUILD_VERSION%.zip
)

popd
