@echo off

pushd %~dp0

docker build -t xpiksftp .

rem yes, should always be started from xpiks root
SET ROOT_DIR="%cd%"
set FTP_ROOT="ROOT_DIR%\src\xpiks-tests\ftp-for-tests"

docker run -p 20-21:20-21 -p 10090-10100:10090-10100 -v %ROOT_DIR%:/home/ftpuser -d --name flying_unicorn xpiksftp

popd
