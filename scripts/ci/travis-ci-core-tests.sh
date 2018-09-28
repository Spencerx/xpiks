#!/bin/bash

echo "----------------------"
echo "Building Core tests..."
echo "----------------------"

pushd src/xpiks-tests/xpiks-tests-core/

qmake "CONFIG+=debug travis-ci" xpiks-tests-core.pro
make

echo "Building Core tests... Done"

if [ "${TRAVIS_OS_NAME}" = "linux" ]; then
    export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:../../../libs/debug/
elif [ "${TRAVIS_OS_NAME}" = "osx" ]; then
    export DYLD_LIBRARY_PATH=$DYLD_LIBRARY_PATH:../../../libs/debug/
fi

./xpiks-tests-core
exitcode=$?

if [ $exitcode != 0 ]; then

    if [ "${TRAVIS_OS_NAME}" = "linux" ]; then
        gdb $(pwd)/xpiks-tests-integration core* -ex "thread apply all bt" -ex "set pagination 0" -batch
    elif [ "${TRAVIS_OS_NAME}" = "osx" ]; then
        lldb -c /cores/core.xpiks* --batch -o 'thread backtrace all' -o 'quit'
    fi

    exit $exitcode
fi

"$TRAVIS_BUILD_DIR/scripts/ci/get_code_cov.sh" "$PWD/../../"
bash <(curl -s https://codecov.io/bash) -X gcov -F "${TRAVIS_OS_NAME}" || echo 'Codecov failed to upload';

popd

