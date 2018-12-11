#!/bin/bash

set -x

if [ "${TRAVIS_OS_NAME}" != "linux" ]; then
    exit 1
fi

source /opt/qt56/bin/qt56-env.sh

# ccache
sudo /usr/sbin/update-ccache-symlinks
export PATH="/usr/lib/ccache:$PATH"

if [ -n "$UI_TESTS" ]; then
    export DISPLAY=:99.0
    sh -e /etc/init.d/xvfb start
    sleep 3
fi

if [ -n "$INTEGRATION_TESTS" ]; then
    sudo mkdir -v --mode 0777 -p /var/ftp/incoming
    # vsftpd wants root to be read only!
    sudo chmod -v ugo-w /var/ftp
    sudo chown -v -R ftp:ftp /var/ftp
    sudo cp -v "${TRAVIS_BUILD_DIR}/scripts/ci/my_vsftpd.conf" /etc/vsftpd.conf
    sudo service vsftpd restart
fi

echo "Done"
