#!/bin/bash

if [ "${TRAVIS_OS_NAME}" = "linux" ]; then
    if [ "$CXX" = "g++" ]; then export CXX="g++-5" CC="gcc-5"; fi
    source /opt/qt56/bin/qt56-env.sh
fi
