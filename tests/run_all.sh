#!/usr/bin/env bash
set -u

EXE=${1:-./interpreter}

if [ ! -x "$EXE" ]; then
    echo "Executable not found: $EXE"
    echo "Usage: ./run_all.sh ./interpreter"
    exit 1
fi

for test in tests/*.txt; do
    echo "=============================="
    echo "Running $test"
    "$EXE" "$test"
    status=$?
    if [ $status -ne 0 ]; then
        echo "FAILED: $test, exit code $status"
        exit $status
    fi
    echo "PASSED: $test"
    echo
 done

echo "All tests finished"
