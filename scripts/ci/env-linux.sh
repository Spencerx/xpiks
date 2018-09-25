#!/bin/bash

if [ "${TRAVIS_OS_NAME}" != "linux" ]; then
    exit 1
fi

source /opt/qt56/bin/qt56-env.sh

if [ -n "$UI_TESTS" ]; then
    export DISPLAY=:99.0
    sh -e /etc/init.d/xvfb start
    sleep 3
fi

