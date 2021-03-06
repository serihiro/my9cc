#!/bin/bash
try() {
    expected="$1"
    input="$2"

    ./9cc "$input" > tmp.s
    gcc tmp.s -o tmp functions.o
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
try 1 '1 * 2 == 2;'
try 1 'a=1;b=1;a==b;'
try 0 'a=1;b=2;a==b;'
try 1 'a=1; b = 2;a !=  b;'
try 128 'foo();'
try 3 'bar(1,2);'
try 20 'bar((1+2)*4,8);'
try 2 'a=1;b=1;return a+b;3;'
try 2 'if(1==1)return 2;'
try 2 'if(1==1)return 2;return 3;'
try 3 'if(1==0)return 2;return 3;'
try 3 'a=1;b=2;if(a==b)return 2;return 3;'
try 3 'a=1;b=2;if(a==b)return 2;if(1==1)return 3;return 4;'
try 2 'if(1==1)return 2;else return 3;'
try 3 'if(1==0)return 2;else return 3;'
try 3 'if(1==0)return 2;else if(1==1)return 3;'
try 4 'if(1==0)return 2;else if(1==0)return 3;else return 4;'
try 55 'a=0;while(a!=55) a=a+1;'
try 10 'b=0;for(a=0;a!=10;a=a+1)b=b+1;return b;'
try 10 'a=0;b=0;for(;a!=10;b=b+1)a=a+1;return b;'
try 10 'a=0;for(;a!=10;)a=a+1;return a;'
try 200 'b=0;for(a=0;a!=10;a=a+1)if(a==8)b=100;else b=200;return b;'
try 2 'if(1==1){a=1;b=1;}return a+b;'
try 31 'a=0;b=0;c=0;while(a!=10) {a=a+1;b=b+2;}c=c+1;return a+b+c;'
try 15 'b=0;for(a=0;a!=10;a=a+1){b=b+1;a=a+1;}return a+b;'
try 10 'a=0;for(;a!=10;a=a+1){}return a;'
try 0 '{a=0;b=1;c=2;}return c;'

echo OK

