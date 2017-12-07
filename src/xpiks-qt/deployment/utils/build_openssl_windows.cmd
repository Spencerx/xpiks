rem prerequisites: Perl, Visual Studio 2015
rem start Visual Studio cmdline for x64
rem navigate to the openssl 1.0.2m downloaded and extracted

mkdir _build
perl Configure VC-WIN64A --prefix=_build
ms\do_win64a.bat
nmake -f ms\ntdll.mak
nmake -f ms\ntdll.mak install