#!/bin/sh

set -eu

step=$(sed -n '/^program_name/s/.*= *//p' Makefile)
failed=0
temp=usertest

run_test() {
    case "${1##*/}" in
        step4_testcase.*) input="2 4 25 17 6 32 1 4 15 4";;
        test_mult.*)      input="3 2";;
    esac

    echo "${input-}" | ./build/emulator "$1"
}

[ -d "$temp" ] && rm -rf "$temp"

mkdir -p "$temp/actual"
for file in tests/"$step"/inputs/*
do
    filename=${file##*/}
    filename=${filename%.micro}

    echo "Generating code for input file $file"
    output=""

    ./Micro "$file" >"$temp/actual/$filename.asm"
done


mkdir -p "$temp/expected"
for file in tests/"$step"/outputs/*
do
    filename=${file##*/}
    filename=${filename%.*}

    echo "Expected output generated for $file"
    run_test "$file" >"$temp/expected/$filename.out"
done

# Checking your outputs
for file in "$temp"/actual/*.asm
do
    filename=${file##*/}
    filename=${filename%.asm}

    echo -e "\n\n***Testing output of $file***"
    run_test "$file" >"$temp/actual/$filename.out"

    if ! diff -y -B -b "$temp/actual/$filename.out" "$temp/expected/$filename.out"
    then
        failed=$(( failed + 1 ))
    fi
done

exit $failed
