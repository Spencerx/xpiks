#!/bin/bash

echo "Calculating code coverage for directory: $1"

for filename in `find $1 | egrep '\.cpp'`; 
do
    echo "Processing $filename"
    gcov-5 -o . "$filename" > /dev/null; 
done
