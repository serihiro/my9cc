#!/bin/bash
try() {
    expected="$1"
    input="$2"

    ./9cc "$input" > tmp.s
    gcc -o tmp tmp.s
     ./tmp
    actual="$?"

    if [ "$actual" = "$expected" ]; then
        echo "$input => $actual"
    else
        echo "$expected expected, but got $actual"
    exit 1
    fi
}

try 0 0
try 42 42
try 21 '5+20-4'
try 21 ' 5 + 20 - 4 '
try 41 ' 12 + 34 - 5 '
try 12 '3*4'
try 12 ' 3 *    4'
try 24 '4 * 6'
try 24 '6 * 4'
try 8 '3 * 2 + 2'
try 16 '1 + 3 * 5'
try 2 '6 / 3'
try 3 '6 / 3 + 1'
try 6 '21 / 7 * 2'
try 10 '21 / 7 * 2 + 4'

echo OK

