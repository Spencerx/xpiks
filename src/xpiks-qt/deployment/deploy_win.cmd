echo 'Starting deployment...'

set XPIKS_PLATFORM=x64
set XPIKS_VERSION=1.5.0.beta

set XPIKS_DEPS_PATH=%cd%\..\..\..\..\xpiks-deps
set WINDOWS_LIBS=%XPIKS_DEPS_PATH%\windows-libs\release-%XPIKS_PLATFORM%
set REDIST_LIBS=%XPIKS_DEPS_PATH%\windows-libs\redist-%XPIKS_PLATFORM%
set OPENSSL_LIBS=%XPIKS_DEPS_PATH%\windows-libs\openssl-%XPIKS_PLATFORM%
set FFMPEG_LIBS=%XPIKS_DEPS_PATH%\windows-libs\ffmpeg-%XPIKS_PLATFORM%\release

pushd ..\..\build-xpiks-qt-Desktop_Qt_5_9_3_MSVC2015_64bit-Release\release

windeployqt.exe --release --verbose=2 --qmldir=../../xpiks-qt/CollapserTabs/ --qmldir=../../xpiks-qt/Components/ --qmldir=../../xpiks-qt/Constants/ --qmldir=../../xpiks-qt/Dialogs/ --qmldir=../../xpiks-qt/StackViews/ --qmldir=../../xpiks-qt/StyledControls/ --qmldir=../../xpiks-qt/ xpiks-qt.exe

echo 'Copying libraries...'

xcopy /Y /s %WINDOWS_LIBS% .
rem xcopy /Y /s %REDIST_LIBS% .

copy /Y %OPENSSL_LIBS%\*.dll .
copy /Y %FFMPEG_LIBS%\*.dll .
set XPIKS_LIBS_DIR=..\..\..\libs\release

rem copy /Y %XPIKS_LIBS_DIR%\libexiv2.dll .
copy /Y %XPIKS_LIBS_DIR%\libcurl*.dll .
copy /Y %XPIKS_LIBS_DIR%\quazip*.dll .
copy /Y %XPIKS_LIBS_DIR%\ssdll.dll .
copy /Y %XPIKS_LIBS_DIR%\mman.dll .
copy /Y %XPIKS_LIBS_DIR%\face.dll .
rem copy /Y %XPIKS_LIBS_DIR%\libexpat.dll .
copy /Y %XPIKS_LIBS_DIR%\z.dll .
copy /Y %XPIKS_LIBS_DIR%\thmbnlr.dll .
rem -------------------------------
copy /Y ..\..\..\..\ministaller\src\ministaller.exe .

del *.obj
del *.cpp
del *.lib
del *.pdb
del vcredist_%XPIKS_PLATFORM%.exe
del libexiv2.dll
del libexpat.dll

cd ..

echo 'Packing binaries...'

ren release Xpiks
"C:\Program Files\7-Zip\7z" a xpiks-qt-v%XPIKS_VERSION%.zip Xpiks

ren Xpiks xpiks-qt-v%XPIKS_VERSION%-tmp

popd

echo 'Done'
