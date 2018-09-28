#!/bin/bash

echo "-----------------------------"
echo "Building Integration tests..."
echo "-----------------------------"

pushd src/xpiks-tests/plugins-for-tests/helloworld/xpiks-helloworld-plugin/
qmake "CONFIG+=debug travis-ci" xpiks-helloworld-plugin.pro
make
popd

pushd src/xpiks-tests/xpiks-tests-integration
qmake "CONFIG+=debug travis-ci without-video" xpiks-tests-integration.pro
make

echo "Building Integration tests... Done"

set -x

#sysctl -w kern.corefile=core.%P

if [ "${TRAVIS_OS_NAME}" = "linux" ]; then
    export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:../../../libs/debug/
elif [ "${TRAVIS_OS_NAME}" = "osx" ]; then
    export DYLD_LIBRARY_PATH=$DYLD_LIBRARY_PATH:../../../libs/debug/
fi

./xpiks-tests-integration > tests.log
exitcode=$?

if [ $exitcode != 0 ]; then
    cat tests.log | grep -v " debug "

    if [ "${TRAVIS_OS_NAME}" = "linux" ]; then
        gdb $(pwd)/xpiks-tests-integration core* -ex "thread apply all bt" -ex "set pagination 0" -batch
    elif [ "${TRAVIS_OS_NAME}" = "osx" ]; then
        lldb -c /cores/core.xpiks* --batch -o 'thread backtrace all' -o 'quit'
    fi

    exit $exitcode
fi

./xpiks-tests-integration --in-memory > tests_in_memory.log
exitcode=$?

if [ $exitcode != 0 ]; then
    # travis doesn't like long logs
    cat tests_in_memory.log | grep -v " debug "

    if [ "${TRAVIS_OS_NAME}" = "linux" ]; then
        gdb $(pwd)/xpiks-tests-integration core* -ex "thread apply all bt" -ex "set pagination 0" -batch
    elif [ "${TRAVIS_OS_NAME}" = "osx" ]; then
        lldb -c /cores/core.xpiks* --batch -o 'thread backtrace all' -o 'quit'
    fi

    exit $exitcode
fi

cat tests.log | grep -v " debug "

"$TRAVIS_BUILD_DIR/scripts/ci/get_code_cov.sh" "$PWD/../../"
bash <(curl -s https://codecov.io/bash) -X gcov -F "${TRAVIS_OS_NAME}" || echo 'Codecov failed to upload';

popd
