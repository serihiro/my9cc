#!/bin/bash

make

gcc functions.c -o functions -c

./9cc "foo();" > tmp.s
gcc tmp.s functions -o tmp
./tmp

./9cc "bar(1,2);" > tmp.s
gcc tmp.s functions -o tmp
./tmp
