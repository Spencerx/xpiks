@echo off
rem -----------------------------------
rem TO BE RUN FROM THE ROOT OF THE REPO
rem -----------------------------------

set BUILD_MODE=%1
if "%BUILD_MODE%"=="debug" set TARGET="debug"
if "%BUILD_MODE%"=="appveyor" set TARGET="debug"
if "%BUILD_MODE%"=="release" set TARGET="release"
if "%BUILD_MODE%"=="fulldebug" set TARGET="debug"
if "%BUILD_MODE%"=="fullrelease" set TARGET="release"

set ARCH=x64

SET ROOT_DIR="%cd%"
echo "Root directory is %ROOT_DIR%"
echo "Build mode is %BUILD_MODE%"

rem for Windows 10+ only
rem set PRINT_PREFIX="^<ESC^>[32m [32mGreen[0m"
set PRINT_PREFIX="-"
set NMAKE_OPTIONS="clean all /f Makefile"

set XPIKS_DEPS_ROOT="%ROOT_DIR%\..\xpiks-deps"
set XPIKS_DEPS_LIBS="%XPIKS_DEPS_ROOT%\windows-libs\%TARGET%-%ARCH%"
set LIBS_DIR="%ROOT_DIR%\libs\%TARGET%"

if "%BUILD_MODE%"=="appveyor" (
   set PRINT_PREFIX=""
   set QMAKE_EXE="qmake"
   rem call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" %ARCH%
   git submodule update --init --recursive
) else (
  set QMAKE_EXE="C:\Qt\Qt5.6.2\5.6\msvc2015_64\bin\qmake.exe"
)

rem chillout
echo "%PRINT_PREFIX% Building chillout..."
cd %ROOT_DIR%\vendors\chillout\src\chillout
%QMAKE_EXE% "CONFIG+=%TARGET%" chillout.pro
nmake.exe "%NMAKE_OPTIONS%"
copy %TARGET%\chillout.* %LIBS_DIR%
echo "%PRINT_PREFIX% Building chillout... - done"

rem zlib
echo "%PRINT_PREFIX% Building zlib..."
move %XPIKS_DEPS_ROOT%\zlib-1.2.11 %ROOT_DIR%\vendors
cd %ROOT_DIR%\vendors\zlib-project
%QMAKE_EXE% "CONFIG+=%TARGET%" zlib.pro
nmake.exe "%NMAKE_OPTIONS%"
copy %TARGET%\z.* %LIBS_DIR%
echo "%PRINT_PREFIX% Building zlib... - done"

rem mman
echo "%PRINT_PREFIX% Building mman..."
cd %ROOT_DIR%\vendors\cpp-libface\mman-win32
%QMAKE_EXE% "CONFIG+=%TARGET%" mman-win32.pro
nmake.exe "%NMAKE_OPTIONS%"
copy %TARGET%\mman.* %LIBS_DIR%
echo "%PRINT_PREFIX% Building mman... - done"

rem tiny-aes
echo "%PRINT_PREFIX% Renaming tiny-aes..."
cd %ROOT_DIR%\vendors\tiny-aes
ren aes.c aes.cpp
echo "%PRINT_PREFIX% Renaming tiny-aes... - done"

rem cpp-libface
echo "%PRINT_PREFIX% Building cpp-libface..."
cd %ROOT_DIR%\vendors\cpp-libface\libface-project
%QMAKE_EXE% "CONFIG+=%TARGET%" libface.pro
nmake.exe "%NMAKE_OPTIONS%"
copy %TARGET%\face.* %LIBS_DIR%
echo "%PRINT_PREFIX% Building cpp-libface... - done"

rem ssdll
echo "%PRINT_PREFIX% Building ssdll..."
cd %ROOT_DIR%\vendors\ssdll\src\ssdll
%QMAKE_EXE% "CONFIG+=%TARGET% %BUILD_MODE%" ssdll.pro
nmake.exe "%NMAKE_OPTIONS%"
copy %TARGET%\ssdll.* %LIBS_DIR%
echo "%PRINT_PREFIX% Building ssdll... - done"

rem quazip
echo "%PRINT_PREFIX% Building quazip..."
cd %ROOT_DIR%\vendors\quazip\quazip
%QMAKE_EXE% "CONFIG+=%TARGET%" quazip.pro
nmake.exe "%NMAKE_OPTIONS%"
copy %TARGET%\quazip* %LIBS_DIR%
echo "%PRINT_PREFIX% Building quazip... - done"

rem hunspell
echo "%PRINT_PREFIX% Building hunspell..."
cd %ROOT_DIR%\vendors\hunspell
%QMAKE_EXE% "CONFIG+=%TARGET%" hunspell.pro
nmake.exe "%NMAKE_OPTIONS%"
copy %TARGET%\hunspell.* %LIBS_DIR%
echo "%PRINT_PREFIX% Building hunspell... - done"

echo "%PRINT_PREFIX% Building recoverty..."
cd %ROOT_DIR%\src\recoverty
%QMAKE_EXE% "CONFIG+=%TARGET%" recoverty.pro
nmake.exe "%NMAKE_OPTIONS%"
mkdir ..\xpiks-qt\deps\recoverty
copy %TARGET%\* ..\xpiks-qt\deps\recoverty\
echo "%PRINT_PREFIX% Building recoverty... - done"

set XPKS_ROOT="%ROOT_DIR%\..\libxpks"
set THMBNLR_ROOT="%ROOT_DIR%\..\libthmbnlr"

if not "%BUILD_MODE%"=="appveyor" (
  set "TRUE="
  IF "%BUILD_MODE%"=="fulldebug" set TRUE=1
  IF "%BUILD_MODE%"=="fullrelease" set TRUE=1
  IF defined TRUE (
      rem libxpks
      echo "%PRINT_PREFIX% Building real libxpks..."
      copy %ROOT_DIR%\libs\%TARGET%\*curl* %XPKS_ROOT%\libs\%TARGET%
      copy %ROOT_DIR%\libs\%TARGET%\quazip* %XPKS_ROOT%\libs\%TARGET%
      cd %XPKS_ROOT%\src\xpks
      %QMAKE_EXE% "CONFIG+=%TARGET%" xpks.pro
      nmake.exe "%NMAKE_OPTIONS%"
      copy %TARGET%\xpks.* %LIBS_DIR%
      echo "%PRINT_PREFIX% Building libxpks... - done"

      rem libthmbnlr
      echo "%PRINT_PREFIX% Building real libthmbnlr..."
      cd "%THMBNLR_ROOT%\src\libthmbnlr"
      %QMAKE_EXE% "CONFIG+=%TARGET%" libthmbnlr.pro
      nmake.exe "%NMAKE_OPTIONS%"
      copy %TARGET%\thmbnlr.* %LIBS_DIR%
      echo "%PRINT_PREFIX% Building libthmbnlr... - done"	  
    ) else (
      rem libxpks
      echo "%PRINT_PREFIX% Building stub libxpks..."
      cd %ROOT_DIR%\src\libxpks_stub
      %QMAKE_EXE% "CONFIG+=%TARGET%" libxpks_stub.pro
      nmake.exe "%NMAKE_OPTIONS%"
      copy %TARGET%\xpks.* %LIBS_DIR%
      echo "%PRINT_PREFIX% Building libxpks... - done"

      rem libthmbnlr
      echo "%PRINT_PREFIX% Building stub libthmbnlr..."
      cd "%ROOT_DIR%\vendors\libthmbnlr"
      %QMAKE_EXE% "CONFIG+=%TARGET%" thmbnlr.pro
      nmake.exe "%NMAKE_OPTIONS%"
      copy %TARGET%\thmbnlr.* %LIBS_DIR%
      echo "%PRINT_PREFIX% Building libthmbnlr... - done"
    )
) else (
  echo "%PRINT_PREFIX% Copying non-buildable artifacts from xpiks-deps..."
  echo "%PRINT_PREFIX% xpiks-deps root is %XPIKS_DEPS_LIBS%"

  rem ffmpeg
  xcopy /s %XPIKS_DEPS_ROOT%\windows-libs\ffmpeg-%ARCH%\%TARGET% %LIBS_DIR%
  
  rem xpiks deps
  copy %XPIKS_DEPS_LIBS%\*curl* %LIBS_DIR%
  copy %XPIKS_DEPS_LIBS%\thmbnlr.* %LIBS_DIR%
  copy %XPIKS_DEPS_LIBS%\xpks.* %LIBS_DIR%
  rem integration tests deps
  copy %XPIKS_DEPS_LIBS%\libexpat.* %LIBS_DIR%
  copy %XPIKS_DEPS_LIBS%\libexiv2.* %LIBS_DIR%
)

echo "%PRINT_PREFIX% Vendors preparation for %BUILD_MODE%: done"
