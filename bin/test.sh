#!/bin/bash

set -eu

step=step2
failed=0
program=${1:?Please provide the executable}

for file in tests/$step/inputs/*
do
    filename=${file##*/}
    filename=${filename%.micro}

    input=./tests/$step/inputs/${filename}.micro
    output=./tests/$step/outputs/${filename}.out

    if ! "$program" "$input" | diff -b "$output" -
    then
        (( ++failed ))
        echo "Failed test $filename"
    fi
done

if (( failed == 0 ))
then
    echo Success
else
    echo
    echo "Failed $failed tests."
fi
