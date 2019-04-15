#!/bin/sh

step=$(sed -n '/^program_name/s/.*= *//p' Makefile)
./build/$step $1 #2>/dev/null
