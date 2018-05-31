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

export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:../../../libs/debug/

./xpiks-tests-integration > tests.log
exitcode=$?

if [ $exitcode != 0 ]; then
    cat tests.log
    
    for i in $(find ./ -maxdepth 1 -name 'core*' -print); do
        gdb $(pwd)/xpiks-tests-integration core* -ex "thread apply all bt" -ex "set pagination 0" -batch
    done

    exit $exitcode
fi

./xpiks-tests-integration --in-memory > tests_in_memory.log
exitcode=$?

if [ $exitcode != 0 ]; then
    cat tests_in_memory.log
    
    for i in $(find ./ -maxdepth 1 -name 'core*' -print); do
        gdb $(pwd)/xpiks-tests-integration core* -ex "thread apply all bt" -ex "set pagination 0" -batch
    done;

    exit $exitcode
fi

cat tests.log

"$TRAVIS_BUILD_DIR/scripts/ci/get_code_cov.sh" "$PWD/../../"
bash <(curl -s https://codecov.io/bash) -X gcov || echo 'Codecov failed to upload';

popd
