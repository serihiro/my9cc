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

try 0 '0;'
try 42 '42;'
try 21 '5+20-4;'
try 21 ' 5 + 20 - 4 ;'
try 41 ' 12 + 34 - 5;'
try 12 '3*4;'
try 12 ' 3 *    4;'
try 24 '4 * 6;'
try 24 '6 * 4;'
try 8 '3 * 2 + 2;'
try 16 '1 + 3 * 5;'
try 2 '6 / 3;'
try 3 '6 / 3 + 1;'
try 6 '21 / 7 * 2;'
try 10 '21 / 7 * 2 + 4;'
try 18 '21 / 7 * (2 + 4);'
try 7 '   ( 21 / 7) + 4;'
try 10 '(21-7) - 4;'
try 170 '1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1;'
try 5 '1+1;4+1;'
try 3 'a=3;a;'
try 3 'a=b=3;b;c=4;a;'
try 7 'a=b=3;b=2;c=4;a+c;'
try 10 'a=7;b=3;a+b;'
try 21 'a=7;b=3;a*b;'
try 100 '  a=2;b  =5;c=  10 ; a * b *  c  ; '
try 102 'a=2;b  =5;c=  10 ;d =  a * b *  c  ;d+2; '
try 3 'A=3;A;'
try 2 'ab=1;cd=1;ab+cd;'
try 9 'ab1=4;cd2=5;ab1+cd2;'
try 9 'gasdfsadfSADFWERdgadsfsd5fsd4afs4adfb=4;cd2=5;gasdfsadfSADFWERdgadsfsd5fsd4afs4adfb+cd2;'
try 1 '1==1;'
try 0 '2==1;'
try 1 '2!=1;'
try 1 'a=1;b=1;a==b;'
try 0 'a=1;b=2;a==b;'
try 1 'a=1; b = 2;a !=  b;'

echo OK

