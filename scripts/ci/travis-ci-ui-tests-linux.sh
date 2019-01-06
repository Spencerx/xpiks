#!/bin/bash

echo "--------------------"
echo "Building UI tests..."
echo "--------------------"

scripts/docker/vsftpd/cleanup.sh
scripts/docker/vsftpd/start.sh

# for csv export test
mkdir ~/Downloads

pushd src/xpiks-tests/xpiks-tests-ui

qmake "CONFIG+=debug travis-ci" xpiks-tests-ui.pro
make -j$(nproc)

echo "Building UI tests... Done"
echo "Starting UI tests..."

export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:../../../libs/debug/

./xpiks-tests-ui > uitests.log
exitcode=$?

if [ $exitcode != 0 ]; then
    cat uitests.log

    for i in $(find ./ -maxdepth 1 -name 'core*' -print); do
        gdb $(pwd)/xpiks-tests-ui core* -ex "thread apply all bt" -ex "set pagination 0" -batch
    done

    exit $exitcode
fi

cat uitests.log

"$TRAVIS_BUILD_DIR/scripts/ci/get_code_cov.sh" "$PWD/../../"
bash <(curl -s https://codecov.io/bash) -X gcov || echo 'Codecov failed to upload';

popd
