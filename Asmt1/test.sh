#!/bin/bash

# Trigger all your test cases with this script
export ASAN_OPTIONS=verify_asan_link_order=0

cd ./tests
for args in *.args
do  
    ((i++))
    cat "$(basename "$args" | sed 's/\(.*\)\..*/\1/')".in | xargs -a $args ../timetable >"$(basename "$args" | sed 's/\(.*\)\..*/\1/')".actual
    if diff "$(basename "$args" | sed 's/\(.*\)\..*/\1/')".actual "$(basename "$args" | sed 's/\(.*\)\..*/\1/')".out
    then
        echo $i, $args pass the testcase!
    else 
        echo $i, Error occur! $args did not pass!
    fi
    echo ""
done