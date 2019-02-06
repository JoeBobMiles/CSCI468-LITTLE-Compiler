#!/bin/bash

for file in $(ls ./tests/inputs/)
do
	filename=$(echo $file | sed -En 's/^(.*)\.micro/\1/p')
	./build/test.exe ./tests/inputs/${filename}.micro | diff -b ./tests/outputs/${filename}.out -

	if [[ $? -ne 0 ]]
	then
		echo "Failed test $filename"
	fi
done
