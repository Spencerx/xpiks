#!/bin/bash

echo "-----------------"
echo "Building Xpiks..."
echo "-----------------"

pushd src/xpiks-qt/

qmake "CONFIG+= debug travis-ci without-video" xpiks-qt.pro
make

exitcode=$?

if [ $exitcode != 0 ]; then
    echo "Failed to build Xpiks"
    exit $exitcode
fi

echo "Building Xpiks... Done"

echo "Starting cppcheck..."

cppcheck --language=c++ --inconclusive --quiet -DQT_DEBUG -DWITH_LOGS -DWITH_PLUGINS -DWITH_STDOUT_LOGS -DTRAVIS_CI -UCORE_TESTS -UINTEGRATION_TESTS -UQ_OS_WIN -DQ_OS_LINUX --error-exitcode=1 --suppress=missingIncludeSystem --suppress='*:moc_*.cpp' --max-configs=15 --enable=warning .

echo "cppcheck exited with code $?"

popd
