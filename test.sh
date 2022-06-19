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

assert 0 'main() {0;}'
assert 42 'main () {42;}'
assert 100 'main() {99+1;}'
assert 123 'main() {100 + 20 + 3;}'
assert 80 'main() {100 - 10 - 10;}'
assert 12 'main() {3+3*3;}'
assert 4 'main() {3+3/3;}'
assert 18 'main() {(3+3)*3;}'
assert 7 'main() {-3+10;}'
assert 1 'main() {5<=10;}'
assert 1 'main() {5<10;}'
assert 0 'main() {5>10;}'
assert 0 'main() {5>=10;}'
assert 0 'main() {5==10;}'
assert 1 'main() {5!=10;}'
assert 4 'main() {a = 4;}'
assert 13 'main() {a = 4;b = 5 * 6 - 8;(a + b) / 2;}'
assert 13 'main() {foo = 4;bar = 5 * 6 - 8;(foo + bar) / 2;}'
assert 5 'main() {foo = 1;bar = 4; return foo+bar;}'
assert 4 'main() {foo = 1; if (foo==1) return 4; return 3;}'
assert 3 'main() {foo = 1; if (foo==0) return 4; else return 3;}'
assert 5 'main() {i = 1;while (i < 5) i = i + 1; return i;}'
assert 5 'main() {foo = 0;for (i = 0; i < 5; i = i + 1) foo = foo + 1;return foo;}'
assert 5 'main() {foo = 0;i = 0;for (; i < 5; i = i + 1) foo = foo + 1;return foo;}'
assert 5 'main() {i=1;for(;i<5;)i=i+1;return i;}'
assert 5 'main() {for(i=1;i<5;)i=i+1;return i;}'
assert 10 'main() {foo=0;bar=0; for(i=0;i<5;i=i+1){foo = foo + 1;bar = bar + 1;} return foo + bar;}'
assert 5 'main() {foo=0;for(i=0;i<5;i+=1)foo+=1; return foo;}'
assert 1 'main() {i=0; if ((i+=1) == 1) return 1; else return 0;}'
assert 3 'main() {a=0;b=&a;*b=3;return a;}'
assert 23 'main() {return foo();}' '#include <stdio.h>
int foo() { printf("\n\nfunction call!!\n\n"); return 23;}'
assert 3 'main() {return foo(3);}' 'int foo(int a) {return a;}'
assert 23 'main() {return foo(20, 3);}' 'int foo(int a, int b) {return a + b;}'
assert 123 'main() {return foo(100, 20, 3);}' 'int foo(int a, int b, int c) {return a + b + c;}'
assert 3 'main() {return foo(1+2);}' 'int foo(int a) {return a;}'
assert 123 'main() {return foo(50+50, 20, 3);}' 'int foo(int a, int b, int c) {return a + b + c;}'
assert 123 'main() {a=100;return foo(a, 20, 3);}' 'int foo(int a, int b, int c) {return a + b + c;}'
assert 123 'foo(a) {return a;} main() {return foo(123);}'
assert 123 'foo(a, b) {return a + b;} main() {return foo(100 + 23);}'

echo OK
