#!/bin/bash

make

gcc functions.c -o functions -c

./9cc "foo();" > tmp.s
gcc tmp.s functions -o tmp
./tmp

./9cc "bar();" > tmp.s
gcc tmp.s functions -o tmp
./tmp
