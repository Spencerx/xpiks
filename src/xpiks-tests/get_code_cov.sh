#!/bin/bash

echo "Calculating code coverage for directory: $1"

for filename in `find $1 -type f \( -name "*.cpp" -not -name "moc_*.cpp" \)`; 
do
    echo "Processing $filename"
    gcov-5 -o . "$filename" > /dev/null; 
done