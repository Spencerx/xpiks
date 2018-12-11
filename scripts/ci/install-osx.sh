#!/usr/bin/env bash

if [ "${TRAVIS_OS_NAME}" != "osx" ]; then
    echo "Not a macOS build; skipping installation"
    exit 0
fi

set -eu

brew update
brew install exiv2
brew install ffmpeg
brew install ctags
brew install cppcheck
brew install exiftool
brew install exempi
brew install qt5
brew install ccache

