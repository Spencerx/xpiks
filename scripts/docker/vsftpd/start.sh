#!/bin/bash

REPO_ROOT=$(git rev-parse --show-toplevel)

pushd "${REPO_ROOT}/scripts/docker/vsftpd"

docker build -t xpiksftp .

FTP_ROOT="${REPO_ROOT}/src/xpiks-tests/ftp-for-tests"

find "${FTP_ROOT}" -maxdepth 1 -type f ! -name '.keep' -exec rm -v {} +

docker run -p 20-21:20-21 -p 10090-10100:10090-10100 -v "${FTP_ROOT}":/home/ftpuser -d --name flying_unicorn xpiksftp

popd
