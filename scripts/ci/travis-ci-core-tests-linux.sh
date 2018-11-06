#!/bin/bash

echo "----------------------"
echo "Building Core tests..."
echo "----------------------"

pushd src/xpiks-tests/xpiks-tests-core/

qmake "CONFIG+=debug travis-ci" xpiks-tests-core.pro
make -j$(nproc)

echo "Building Core tests... Done"

export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:../../../libs/debug/

./xpiks-tests-core
exitcode=$?

if [ $exitcode != 0 ]; then
    for i in $(find ./ -maxdepth 1 -name 'core*' -print); do
        gdb $(pwd)/xpiks-tests-core core* -ex "thread apply all bt" -ex "set pagination 0" -batch
    done
    exit $exitcode
fi

"$TRAVIS_BUILD_DIR/scripts/ci/get_code_cov.sh" "$PWD/../../"
bash <(curl -s https://codecov.io/bash) -X gcov -F "${TRAVIS_OS_NAME}" || echo 'Codecov failed to upload';

popd

