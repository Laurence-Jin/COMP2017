#!/bin/bash

# Trigger all your test cases with this script
export ASAN_OPTIONS=verify_asan_link_order=0

cd ./tests
for args in *.x2017
    do
    ((i++))
    ../objdump_x2017 "$(basename "$args" | sed 's/\(.*\)\..*/\1/')".x2017 > "$(basename "$args" | sed 's/\(.*\)\..*/\1/')".actual
    if diff "$(basename "$args" | sed 's/\(.*\)\..*/\1/')".asm "$(basename "$args" | sed 's/\(.*\)\..*/\1/')".actual
    then
        echo $i, $args pass the testcase!
    else 
        echo $i, Error occur! $args did not pass!
    fi
    echo ""
done