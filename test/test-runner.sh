#!/bin/sh

TESTS=01_stdout

echo "Running the test suite:"

for test in $TESTS; do
    ./${test} ;
    if [ $? = 0 ]; then
	echo "* ${test}: success"
    else
	echo "* ${test}: failed"
    fi
done

