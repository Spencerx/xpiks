#!/bin/bash

set -x


if [ "${TRAVIS_OS_NAME}" != "linux" ]; then
    echo "Not a Linux build; skipping installation"
    exit 0
fi

set -x

sudo add-apt-repository ppa:ubuntu-toolchain-r/test -y
sudo add-apt-repository ppa:beineri/opt-qt562-trusty -y
sudo add-apt-repository ppa:kubuntu-ppa/backports -y
sudo add-apt-repository ppa:jonathonf/ffmpeg-3 -y
sudo apt-get update -qq

export APT_CACHE_DIR="$HOME/.my-apt-cache"
mkdir -pv $APT_CACHE_DIR

sudo apt-get -o dir::cache::archives="$APT_CACHE_DIR" install -y ctags sed
sudo apt-get -o dir::cache::archives="$APT_CACHE_DIR" install -y gcc-5 g++-5 lcov
sudo apt-get install -qq -o dir::cache::archives="$APT_CACHE_DIR" qt56base;
sudo apt-get -o dir::cache::archives="$APT_CACHE_DIR" install qt56base qt56quickcontrols qt56svg qt56declarative qt56graphicaleffects libcurl4-openssl-dev hunspell-en-us zlib1g-dev libimage-exiftool-perl
sudo apt-get -o dir::cache::archives="$APT_CACHE_DIR" install libavcodec-dev libavdevice-dev libavformat-dev libavfilter-dev libavutil-dev libswscale-dev
sudo apt-get -o dir::cache::archives="$APT_CACHE_DIR" install -y ccache

# Update symlinks
sudo /usr/sbin/update-ccache-symlinks
export PATH="/usr/lib/ccache:$PATH"


if [ -n "$XPIKS_BINARY" ]; then
    sudo apt-get -o dir::cache::archives="$APT_CACHE_DIR" install -qq cppcheck
else
    sudo apt-get -o dir::cache::archives="$APT_CACHE_DIR" install -y gdb
fi

if [ -n "$INTEGRATION_TESTS" ]; then
    sudo apt-get -o dir::cache::archives="$APT_CACHE_DIR" install -y vsftpd
    sudo apt-get -o dir::cache::archives="$APT_CACHE_DIR" install -y libexiv2-dev expat
fi

sudo update-alternatives --install /usr/bin/gcov gcov /usr/bin/gcov-5 90
sudo update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-5 90
