#!/bin/bash

echo "--------------------"
echo "Building UI tests..."
echo "--------------------"

pushd src/xpiks-tests/ui-tests-stubs
qmake "CONFIG+=debug travis-ci" ui-tests-stubs.pro
make
popd

pushd src/xpiks-tests/xpiks-tests-ui

qmake "CONFIG+=debug travis-ci" xpiks-tests-ui.pro
make

echo "Building UI tests... Done"
echo "Starting UI tests..."

export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:../../../libs/debug/
./xpiks-tests-ui
exitcode=$?

if [ $exitcode != 0 ]; then
    for i in $(find ./ -maxdepth 1 -name 'core*' -print); do
        gdb $(pwd)/xpiks-tests-ui core* -ex "thread apply all bt" -ex "set pagination 0" -batch
    done

    exit $exitcode
fi

"$TRAVIS_BUILD_DIR/scripts/ci/get_code_cov.sh" "$PWD/../../"
bash <(curl -s https://codecov.io/bash) -X gcov || echo 'Codecov failed to upload';

popd
