#!/bin/bash
assert() {
  expected="$1"
  input="$2"

  ./build/ucc "$input" > tmp.s
  if [ "$#" = 3 ]; then
    echo "$3" > tmp_lib.c
    cc -c tmp_lib.c
    cc -o tmp tmp.s tmp_lib.o
  else
    cc -o tmp tmp.s
  fi
  ./tmp
  actual="$?"

  if [ "$actual" = "$expected" ]; then
    echo "$input => $actual"
  else
    echo "$input => $expected expected, but got $actual"
    exit 1
  fi
}

assert 0 '0;'
assert 42 '42;'
assert 100 '99+1;'
assert 123 '100 + 20 + 3;'
assert 80 '100 - 10 - 10;'
assert 12 '3+3*3;'
assert 4 '3+3/3;'
assert 18 '(3+3)*3;'
assert 7 '-3+10;'
assert 1 '5<=10;'
assert 1 '5<10;'
assert 0 '5>10;'
assert 0 '5>=10;'
assert 0 '5==10;'
assert 1 '5!=10;'
assert 4 'a = 4;'
assert 13 'a = 4;b = 5 * 6 - 8;(a + b) / 2;'
assert 13 'foo = 4;bar = 5 * 6 - 8;(foo + bar) / 2;'
assert 5 'foo = 1;bar = 4; return foo+bar;'
assert 4 'foo = 1; if (foo==1) return 4; return 3;'
assert 3 'foo = 1; if (foo==0) return 4; else return 3;'
assert 5 'i = 1;while (i < 5) i = i + 1; return i;'
assert 5 'foo = 0;for (i = 0; i < 5; i = i + 1) foo = foo + 1;return foo;'
assert 5 'foo = 0;i = 0;for (; i < 5; i = i + 1) foo = foo + 1;return foo;'
assert 5 'i=1;for(;i<5;)i=i+1;return i;'
assert 5 'for(i=1;i<5;)i=i+1;return i;'
assert 10 'foo=0;bar=0; for(i=0;i<5;i=i+1){foo = foo + 1;bar = bar + 1;} return foo + bar;'
assert 5 'foo=0;for(i=0;i<5;i+=1)foo+=1; return foo;'
assert 1 'i=0; if ((i+=1) == 1) return 1; else return 0;'
assert 3 'a=0;b=&a;*b=3;return a;'
assert 23 'return foo();' '#include <stdio.h>
int foo() { printf("\n\nfunction call!!\n\n"); return 23;}'
assert 3 'return foo(3);' 'int foo(int a) {return a;}'
assert 23 'return foo(20, 3);' 'int foo(int a, int b) {return a + b;}'
assert 123 'return foo(100, 20, 3);' 'int foo(int a, int b, int c) {return a + b + c;}'
assert 3 'return foo(1+2);' 'int foo(int a) {return a;}'
assert 123 'return foo(50+50, 20, 3);' 'int foo(int a, int b, int c) {return a + b + c;}'
assert 123 'a=100;return foo(a, 20, 3);' 'int foo(int a, int b, int c) {return a + b + c;}'

echo OK
