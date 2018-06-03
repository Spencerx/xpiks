@echo off
echo 'Starting deployment...'

set XPIKS_PLATFORM=x64
set XPIKS_VERSION=1.5.2
set APP_NAME=Xpiks
set BUILD_CONFIGURATION=release

set XPIKS_ROOT=%cd%\..\..
set XPIKS_DEPS_PATH=%XPIKS_ROOT%\..\xpiks-deps
set MINISTALLER_PATH=%XPIKS_ROOT%\..\ministaller
set ARTIFACTS_PATH=%XPIKS_ROOT%\src\build-xpiks-qt-Desktop_Qt_5_6_2_MSVC2015_64bit-Release
set SEVEN_ZIP="C:\Program Files\7-Zip\7z"
set QT_BIN_DIR=C:\Qt\Qt5.6.2\5.6\msvc2015_64\bin
set XPIKS_QT_DIR=../../src/xpiks-qt
set WINDEPLOYQTTOOL=%QT_BIN_DIR%\windeployqt.exe

rem deploy dir should be "Xpiks" in order to have it nicely zipped in the end
set DEPLOY_DIR_NAME=Xpiks

if not defined APPVEYOR goto :deploy

echo "Deploying from Appveyor"

set XPIKS_VERSION=%APPVEYOR_BUILD_VERSION%
set BUILD_CONFIGURATION=debug
set SEVEN_ZIP=7z
set WINDEPLOYQTTOOL=windeployqt.exe
set ARTIFACTS_PATH=%XPIKS_ROOT%\src\xpiks-qt

rem -----------------------------------------

:deploy

echo "Cleaning up old artifacts..."
rmdir xpiks-qt-v%XPIKS_VERSION%-tmp
rmdir Xpiks-v%XPIKS_VERSION%-tmp
rmdir %DEPLOY_DIR_NAME%

set WINDOWS_LIBS=%XPIKS_DEPS_PATH%\windows-libs\%BUILD_CONFIGURATION%-%XPIKS_PLATFORM%
set REDIST_LIBS=%XPIKS_DEPS_PATH%\windows-libs\redist-%XPIKS_PLATFORM%
set OPENSSL_LIBS=%XPIKS_DEPS_PATH%\windows-libs\openssl-%XPIKS_PLATFORM%
set FFMPEG_LIBS=%XPIKS_DEPS_PATH%\windows-libs\ffmpeg-%XPIKS_PLATFORM%\%BUILD_CONFIGURATION%

mkdir %DEPLOY_DIR_NAME%
copy /Y %ARTIFACTS_PATH%\%BUILD_CONFIGURATION%\%APP_NAME%.exe %DEPLOY_DIR_NAME%\

%WINDEPLOYQTTOOL% --%BUILD_CONFIGURATION% --verbose=2 --qmldir=%XPIKS_QT_DIR%/CollapserTabs/ --qmldir=%XPIKS_QT_DIR%/Components/ --qmldir=%XPIKS_QT_DIR%/Constants/ --qmldir=%XPIKS_QT_DIR%/Dialogs/ --qmldir=%XPIKS_QT_DIR%/StackViews/ --qmldir=%XPIKS_QT_DIR%/StyledControls/ --qmldir=%XPIKS_QT_DIR%/ %DEPLOY_DIR_NAME%\%APP_NAME%.exe

set SRC_DEPS_DIR=%XPIKS_ROOT%\src\xpiks-qt\deps

echo "Copying additional files..."
xcopy /Y /s %SRC_DEPS_DIR% %DEPLOY_DIR_NAME%\

echo 'Copying libraries...'

xcopy /Y /s %WINDOWS_LIBS% %DEPLOY_DIR_NAME%\
xcopy /Y /s %REDIST_LIBS% %DEPLOY_DIR_NAME%\

copy /Y %OPENSSL_LIBS%\*.dll %DEPLOY_DIR_NAME%\
copy /Y %FFMPEG_LIBS%\*.dll %DEPLOY_DIR_NAME%\

set XPIKS_LIBS_DIR=%XPIKS_ROOT%\libs\%BUILD_CONFIGURATION%

rem copy /Y %XPIKS_LIBS_DIR%\libexiv2.dll .
copy /Y %XPIKS_LIBS_DIR%\libcurl*.dll %DEPLOY_DIR_NAME%\
copy /Y %XPIKS_LIBS_DIR%\quazip*.dll %DEPLOY_DIR_NAME%\
copy /Y %XPIKS_LIBS_DIR%\ssdll.dll %DEPLOY_DIR_NAME%\
copy /Y %XPIKS_LIBS_DIR%\mman.dll %DEPLOY_DIR_NAME%\
copy /Y %XPIKS_LIBS_DIR%\face.dll %DEPLOY_DIR_NAME%\
rem copy /Y %XPIKS_LIBS_DIR%\libexpat.dll .
copy /Y %XPIKS_LIBS_DIR%\z.dll %DEPLOY_DIR_NAME%\
copy /Y %XPIKS_LIBS_DIR%\thmbnlr.dll %DEPLOY_DIR_NAME%\
copy /Y %XPIKS_DEPS_PATH%\windows-3rd-party-bin\exiftool.exe %DEPLOY_DIR_NAME%\
rem -------------------------------
copy /Y %MINISTALLER_PATH%\src\ministaller.exe %DEPLOY_DIR_NAME%\

del %DEPLOY_DIR_NAME%\*.obj
del %DEPLOY_DIR_NAME%\*.cpp
del %DEPLOY_DIR_NAME%\*.lib
if not defined APPVEYOR del %DEPLOY_DIR_NAME%\*.pdb
del %DEPLOY_DIR_NAME%\vcredist_%XPIKS_PLATFORM%.exe
del %DEPLOY_DIR_NAME%\libexiv2.dll
del %DEPLOY_DIR_NAME%\libexpat.dll
del %DEPLOY_DIR_NAME%\dmg-background.jpg
del %DEPLOY_DIR_NAME%\translations\*.ts
rem need to remove exiftool/ dir as well

echo 'Packing binaries...'

%SEVEN_ZIP% a %APP_NAME%-v%XPIKS_VERSION%.zip Xpiks

ren Xpiks %APP_NAME%-v%XPIKS_VERSION%-tmp

echo 'Done'
