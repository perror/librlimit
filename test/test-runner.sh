#!/bin/sh

TESTS='01_io
       02_timeout
       03_notimeout
       04_forbid_syscall
       05_profile_sleep
       06_profile_stack_alloc
       07_profile_malloc'

failed=0
success=0

echo "Running the test suite:"

for test in $TESTS; do
    ./${test} ;
    if [ $? = 0 ]; then
	echo "* ${test}: success"
	success=$((success + 1))
    else
	echo "* ${test}: failed"
	failed=$((failed + 1))
    fi
done

echo
echo "Summary: $((success+failed)) tests have been processed" \
     "($success success, $failed failed)"
