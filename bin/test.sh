#!/bin/sh

set -eu

step=step2
failed=0

for file in tests/$step/inputs/*
do
    filename=${file##*/}
    filename=${filename%.micro}

    input=./tests/$step/inputs/${filename}.micro
    output=./tests/$step/outputs/${filename}.out

    if ! ./Micro.sh "$input" | diff -B -b "$output" -
    then
        (( ++failed ))
        #echo "Failed test $filename"
    fi
done

exit $failed
