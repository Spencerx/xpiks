cd vendors

cd hunspell
qmake "CONFIG+=%configuration% hunspell.pro"
nmake.exe
cd ..

cd cpp-libface\libface-project
qmake "CONFIG+=%configuration%" libface.pro
nmake.exe
copy %configuration%\face.* ..\..\..\libs\%configuration%
cd ..\..

cd ssdll\src\ssdll
qmake "CONFIG+=%configuration% appveyor" ssdll.pro
nmake.exe
copy %configuration%\ssdll.* ..\..\..\..\libs\%configuration%


rem cd ..\..\..\..\src
