#!/bin/bash

echo "----------------------"
echo "Building Core tests..."
echo "----------------------"

pushd src/xpiks-tests/xpiks-tests-core/

qmake "CONFIG+=debug travis-ci" xpiks-tests-core.pro
make -j$(nproc)

echo "Building Core tests... Done"

export DYLD_LIBRARY_PATH=$DYLD_LIBRARY_PATH:../../../libs/debug/

./xpiks-tests-core &
pid=$!
wait $pid
exitcode=$?

if [ $exitcode != 0 ]; then
    lldb -c /cores/core.$pid --batch -o 'thread backtrace all' -o 'quit'
    exit $exitcode
fi

"$TRAVIS_BUILD_DIR/scripts/ci/get_code_cov.sh" "$PWD/../../"
bash <(curl -s https://codecov.io/bash) -X gcov -F "${TRAVIS_OS_NAME}" || echo 'Codecov failed to upload';

popd

