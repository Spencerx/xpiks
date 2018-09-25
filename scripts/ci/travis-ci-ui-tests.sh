#!/bin/bash

echo "--------------------"
echo "Building UI tests..."
echo "--------------------"

pushd src/xpiks-tests/xpiks-tests-ui

qmake "CONFIG+=debug travis-ci" xpiks-tests-ui.pro
make

echo "Building UI tests... Done"
echo "Starting UI tests..."

if [ "${TRAVIS_OS_NAME}" = "linux" ]; then
    export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:../../../libs/debug/
elif [ "${TRAVIS_OS_NAME}" = "osx" ]; then
    export DYLD_LIBRARY_PATH=$DYLD_LIBRARY_PATH:../../../libs/debug/
fi

./xpiks-tests-ui > uitests_in_memory.log
exitcode=$?

if [ $exitcode != 0 ]; then
    cat uitests_in_memory.log
    
    for i in $(find ./ -maxdepth 1 -name 'core*' -print); do
        gdb $(pwd)/xpiks-tests-ui core* -ex "thread apply all bt" -ex "set pagination 0" -batch
    done

    exit $exitcode
fi

cat uitests_in_memory.log

"$TRAVIS_BUILD_DIR/scripts/ci/get_code_cov.sh" "$PWD/../../"
bash <(curl -s https://codecov.io/bash) -X gcov || echo 'Codecov failed to upload';

popd
