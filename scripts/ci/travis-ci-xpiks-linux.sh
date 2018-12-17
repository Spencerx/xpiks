#!/bin/bash

echo "-----------------"
echo "Building Xpiks..."
echo "-----------------"

pushd src/xpiks-qt/

qmake "CONFIG += debug travis-ci without-video" xpiks-qt.pro
make -j$(nproc)

exitcode=$?

if [ $exitcode != 0 ]; then
    echo "Failed to build Xpiks"
    exit $exitcode
fi

echo "Building Xpiks... Done"

echo "Starting cppcheck..."

cppcheck --version
cppcheck --language=c++ --std=c++11 -j $(nproc) --inconclusive --quiet -DQT_DEBUG -DWITH_LOGS -DWITH_PLUGINS -DWITH_STDOUT_LOGS -DTRAVIS_CI -DQ_OS_LINUX -UCORE_TESTS -UINTEGRATION_TESTS -UUI_TESTS -UQ_OS_WIN --error-exitcode=1 --suppress=missingIncludeSystem --suppress='*:moc_*.cpp' --verbose --enable=warning --showtime=summary --max-configs=5 .

echo "cppcheck exited with code $?"

popd

echo "Checking style guidelines..."

./scripts/contrib/check_style.sh ./src
exitcode=$?

if [ $exitcode != 0 ]; then
    echo "Style check failed"
    exit $exitcode
fi

echo "Done"
