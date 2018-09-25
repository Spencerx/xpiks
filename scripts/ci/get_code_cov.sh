#!/bin/bash

echo "Calculating code coverage for directory: $1"

GCOV_BIN=gcov
if [ "${TRAVIS_OS_NAME}" = "osx" ]; then
    GCOV_BIN=gcov
elif [ "${TRAVIS_OS_NAME}" = "linux" ]; then
    GCOV_BIN=gcov-5
fi

for filename in `find $1 -type f \( -name "*.cpp" -not -name "moc_*.cpp" \)`; 
do
    echo "Processing $filename"
    $GCOV_BIN -o . "$filename" > /dev/null; 
done
