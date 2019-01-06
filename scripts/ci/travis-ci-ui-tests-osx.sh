#!/bin/bash

echo "--------------------"
echo "Building UI tests..."
echo "--------------------"

PROCN=$(sysctl -n hw.ncpu)

pushd src/xpiks-tests/xpiks-tests-ui

qmake "CONFIG+=debug travis-ci" xpiks-tests-ui.pro
make -j$PROCN

echo "Building UI tests... Done"
echo "Starting UI tests..."

export DYLD_LIBRARY_PATH=$DYLD_LIBRARY_PATH:../../../libs/debug/

./xpiks-tests-ui > uitests.log
pid=$!
wait $pid
exitcode=$?

if [ $exitcode != 0 ]; then
    cat uitests.log

    lldb -c /cores/core.$pid --batch -o 'thread backtrace all' -o 'quit'

    exit $exitcode
fi

cat uitests.log

"$TRAVIS_BUILD_DIR/scripts/ci/get_code_cov.sh" "$PWD/../../"
bash <(curl -s https://codecov.io/bash) -X gcov || echo 'Codecov failed to upload';

popd
