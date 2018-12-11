#!/bin/bash

echo "Environment setup"

if [ "${TRAVIS_OS_NAME}" = "osx" ]; then
    CCACHE_PATH="/usr/local/opt/ccache/libexec"
    QT_PATH="/usr/local/opt/qt/bin"
    export PATH="$CCACHE_PATH:$QT_PATH:$PATH"
fi
