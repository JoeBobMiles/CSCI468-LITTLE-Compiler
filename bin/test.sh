#!/bin/bash

set -eu

failed=0

for file in tests/inputs/*
do
    filename=${file##*/}
    filename=${filename%.micro}

    input=./tests/inputs/${filename}.micro
    output=./tests/outputs/${filename}.out

    if ! ./Micro.sh "$input" | diff -B -b -s "$output" -
    then
        (( ++failed ))
        #echo "Failed test $filename"
    fi
done

if (( failed == 0 ))
then
    echo Success
else
    echo
    echo "Failed $failed tests."
fi
