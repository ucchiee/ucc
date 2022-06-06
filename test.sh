#!/bin/bash
assert() {
  expected="$1"
  input="$2"

  ./build/ucc "$input" > tmp.s
  cc -o tmp tmp.s
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
assert 4 'foo = 1; if (foo==1) return 4; else return 3;'

echo OK
