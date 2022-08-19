#!/bin/bash
assert() {
	expected="$1"
	input="$2"

	./build/ucc "$input" >tmp.s
	if [ "$#" = 3 ]; then
		if [[ -e "$3" ]]; then
			cc -c "$3"
			cc -o tmp tmp.s "${3/.c/.o}"
		else
			echo "$3" >tmp_lib.c
			cc -c tmp_lib.c
			cc -o tmp tmp.s tmp_lib.o
		fi
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

assert 0 'int main() {0;}'
assert 42 'int main () {42;}'
assert 100 'int main() {99+1;}'
assert 123 'int main() {100 + 20 + 3;}'
assert 80 'int main() {100 - 10 - 10;}'
assert 12 'int main() {3+3*3;}'
assert 4 'int main() {3+3/3;}'
assert 18 'int main() {(3+3)*3;}'
assert 7 'int main() {-3+10;}'
assert 1 'int main() {5<=10;}'
assert 1 'int main() {5<10;}'
assert 0 'int main() {5>10;}'
assert 0 'int main() {5>=10;}'
assert 0 'int main() {5==10;}'
assert 1 'int main() {5!=10;}'
assert 4 'int main() {int a;a = 4;}'
assert 13 'int main() {int a;int b;a = 4;b = 5 * 6 - 8;(a + b) / 2;}'
assert 13 'int main() {int foo;int bar;foo = 4;bar = 5 * 6 - 8;(foo + bar) / 2;}'
assert 5 'int main() {int foo;int bar;foo = 1;bar = 4; return foo+bar;}'
assert 4 'int main() {int foo;foo = 1; if (foo==1) return 4; return 3;}'
assert 3 'int main() {int foo;foo = 1; if (foo==0) return 4; else return 3;}'
assert 5 'int main() {int i;i = 1;while (i < 5) i = i + 1; return i;}'
assert 5 'int main() {int foo;int i;foo = 0;for (i = 0; i < 5; i = i + 1) foo = foo + 1;return foo;}'
assert 5 'int main() {int foo;int i;foo = 0;i = 0;for (; i < 5; i = i + 1) foo = foo + 1;return foo;}'
assert 5 'int main() {int i;i=1;for(;i<5;)i=i+1;return i;}'
assert 5 'int main() {int i;for(i=1;i<5;)i=i+1;return i;}'
assert 10 'int main() {int foo;int bar;int i;foo=0;bar=0; for(i=0;i<5;i=i+1){foo = foo + 1;bar = bar + 1;} return foo + bar;}'
assert 5 'int main() {int foo;int i;foo=0;for(i=0;i<5;i+=1)foo+=1; return foo;}'
assert 1 'int main() {int i;i=0; if ((i+=1) == 1) return 1; else return 0;}'
assert 3 'int main() {int a;int* b;a=0;b=&a;*b=3;return a;}'
assert 23 'int main() {return foo();}' '#include <stdio.h>
int foo() { printf("\n\nfunction call!!\n\n"); return 23;}'
assert 3 'int main() {return foo(3);}' 'int foo(int a) {return a;}'
assert 23 'int main() {return foo(20, 3);}' 'int foo(int a, int b) {return a + b;}'
assert 123 'int main() {return foo(100, 20, 3);}' 'int foo(int a, int b, int c) {return a + b + c;}'
assert 3 'int main() {return foo(1+2);}' 'int foo(int a) {return a;}'
assert 123 'int main() {return foo(50+50, 20, 3);}' 'int foo(int a, int b, int c) {return a + b + c;}'
assert 123 'int main() {int a;a=100;return foo(a, 20, 3);}' 'int foo(int a, int b, int c) {return a + b + c;}'
assert 123 'int foo(int a) {return a;} int main() {return foo(123);}'
assert 123 'int foo(int a, int b) {return a + b;} int main() {return foo(100, 23);}'
assert 5 'int main() {int a; int b; a = 0; {int a;a = 10; b = a / 2;} return a + b;}'
assert 15 'int main() {int a; int b; a = 0; {int a;a = 10; b = a / 2; return a + b;} return a + b;}'
assert 3 'int main() {int a;int *b;a=0;b=&a;*b=3;return a;}'
assert 23 'int foo(); int main() {return foo();}' 'int foo() {return 23;}'
assert 2 'int main() {int a;int *b;int c;a=0;b=&a;*b=3;c=-*b;return 5+c;}'
assert 2 'int main() {int *p; int *q; myalloc(&p); q = p + 2; return *q;}' '#include <stdlib.h>
void myalloc(int** ptr) { *ptr = (int*)malloc(sizeof(int) * 4); *(*ptr + 0) = 0; *(*ptr + 1) = 1; *(*ptr + 2) = 2; *(*ptr + 3) = 3; } '
assert 1 'int main() {int *p; int *q; myalloc(&p);q=p+2; q=q-1; return *q;}' ' #include <stdlib.h>
void myalloc(int** ptr) { *ptr = (int*)malloc(sizeof(int) * 4); *(*ptr + 0) = 0; *(*ptr + 1) = 1; *(*ptr + 2) = 2; *(*ptr + 3) = 3; } '
assert 2 'int main() {int *p; int *q; myalloc(&p);q=2+p; return *q;}' ' #include <stdlib.h>
void myalloc(int** ptr) { *ptr = (int*)malloc(sizeof(int) * 4); *(*ptr + 0) = 0; *(*ptr + 1) = 1; *(*ptr + 2) = 2; *(*ptr + 3) = 3; } '
assert 4 'int main() {int a; return sizeof(a);}'
assert 8 'int main() {int *a; return sizeof(a);}'
assert 8 'int main() {int *a; return sizeof(a+1);}'
assert 4 'int main() {int *a; return sizeof(*a);}'
assert 4 'int main() {int *a; return sizeof(sizeof(a+1));}'
assert 40 'int main() {int a[10]; return sizeof a;}'
assert 1 'int main() {int a[10]; *a = 1;}'
assert 3 'int main() {int a[2]; int *p; *a = 1; *(a+1) = 2; p = a; return *p + *(p + 1);}'
assert 1 'int main() {int a[10]; a[0] = 1;}'
assert 1 'int main() {int a[10]; 0[a] = 1;}'
assert 0 'int a; int main() {return a;}'
assert 10 'int a; int main() {a = 10; return a;}'
assert 23 'int a[10]; int main() {a[0] = 23; return a[0];}'
assert 24 'int @rc_malloc(int a); int main() {int @a; a = rc_malloc(4); @a = 24; return @a;}' './rc_gc.c'
assert 10 'int @rc_malloc(int a); int main() {int @a; a = rc_malloc(4); a = rc_malloc(4); @a = 10; return @a;}' './rc_gc.c'  # only 8 bytes leaks
assert 11 'int @rc_malloc(int a); int main() {int @a; a = rc_malloc(4); @a = 2; a = rc_malloc(4); @a = 11; return @a;}' './rc_gc.c'  # only 8 bytes leaks

echo OK
