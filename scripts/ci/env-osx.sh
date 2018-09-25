#!/bin/bash

echo "Environment setup"

if [ "${TRAVIS_OS_NAME}" = "osx" ]; then
    export PATH="/usr/local/opt/qt/bin:$PATH"
    echo 'export PATH="/usr/local/opt/qt/bin:$PATH"' >> ~/.bash_profile
fi
