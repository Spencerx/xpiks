echo 'Starting deployment...'

set XPIKS_PLATFORM=x64
set XPIKS_VERSION=1.5.0.beta

rem deploy dir should be "Xpiks" in order to have it nicely zipped in the end
set DEPLOY_DIR_NAME=Xpiks

echo "Cleaning up old artifacts..."
rmdir xpiks-qt-v%XPIKS_VERSION%-tmp
rmdir %DEPLOY_DIR_NAME%

set XPIKS_DEPS_PATH=%cd%\..\..\..\..\xpiks-deps
set WINDOWS_LIBS=%XPIKS_DEPS_PATH%\windows-libs\release-%XPIKS_PLATFORM%
set REDIST_LIBS=%XPIKS_DEPS_PATH%\windows-libs\redist-%XPIKS_PLATFORM%
set OPENSSL_LIBS=%XPIKS_DEPS_PATH%\windows-libs\openssl-%XPIKS_PLATFORM%
set FFMPEG_LIBS=%XPIKS_DEPS_PATH%\windows-libs\ffmpeg-%XPIKS_PLATFORM%\release

mkdir %DEPLOY_DIR_NAME%
copy /Y ..\..\build-xpiks-qt-Desktop_Qt_5_9_3_MSVC2015_64bit-Release\release\xpiks-qt.exe %DEPLOY_DIR_NAME%\

set QT_BIN_DIR=C:\Qt\5.9.3\msvc2015_64\bin

%QT_BIN_DIR%\windeployqt.exe --release --verbose=2 --qmldir=../../xpiks-qt/CollapserTabs/ --qmldir=../../xpiks-qt/Components/ --qmldir=../../xpiks-qt/Constants/ --qmldir=../../xpiks-qt/Dialogs/ --qmldir=../../xpiks-qt/StackViews/ --qmldir=../../xpiks-qt/StyledControls/ --qmldir=../../xpiks-qt/ %DEPLOY_DIR_NAME%\xpiks-qt.exe

echo "Copying additional files..."
xcopy /Y /s ..\deps %DEPLOY_DIR_NAME%\

echo 'Copying libraries...'

xcopy /Y /s %WINDOWS_LIBS% %DEPLOY_DIR_NAME%\
xcopy /Y /s %REDIST_LIBS% %DEPLOY_DIR_NAME%\

copy /Y %OPENSSL_LIBS%\*.dll %DEPLOY_DIR_NAME%\
copy /Y %FFMPEG_LIBS%\*.dll %DEPLOY_DIR_NAME%\
set XPIKS_LIBS_DIR=..\..\..\libs\release

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
copy /Y ..\..\..\..\ministaller\src\ministaller.exe %DEPLOY_DIR_NAME%\

del %DEPLOY_DIR_NAME%\*.obj
del %DEPLOY_DIR_NAME%\*.cpp
del %DEPLOY_DIR_NAME%\*.lib
del %DEPLOY_DIR_NAME%\*.pdb
del %DEPLOY_DIR_NAME%\vcredist_%XPIKS_PLATFORM%.exe
del %DEPLOY_DIR_NAME%\libexiv2.dll
del %DEPLOY_DIR_NAME%\libexpat.dll
del %DEPLOY_DIR_NAME%\dmg-background.jpg

echo 'Packing binaries...'

"C:\Program Files\7-Zip\7z" a xpiks-qt-v%XPIKS_VERSION%.zip Xpiks

ren Xpiks xpiks-qt-v%XPIKS_VERSION%-tmp

echo 'Done'
