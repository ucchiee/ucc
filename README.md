# ucc

Ucc is a (subset of) C compiler, now able to handle this grammar:

```
program       = (type_specifier declarator (";" | compound_stmt))*
type_specifier= "int"
declarator    = ident ( "[" num "]" )?
              | "*" declarator
              | "@" declarator
              | declarator "(" param_decl ("," param_decl)* ")"
param_decl    = type_specifier declarator
stmt          = expr ";"
              | compound_stmt
              | "if" "(" expr ")" stmt ("else" stmt)?
              | "while" "(" expr ")" stmt
              | "for" "(" expr? ";" expr? ";" expr? ")" stmt
              | "return" expr ";"
compound_stmt = "{" ( param_decl ";")* stmt* "}"
expr          = assign
assign        = equality ("=" assign | "+=" assign)?
equality      = relational ("==" relational | "!=" relational)*
relational    = add ("<" add | "<=" add | ">" add | ">=" add)*
add           = mul ("+" mul | "-" mul)*
mul           = unary ("*" unary | "/" unary)*
unary         = ("+" | "-") unary
              | ("*" | "&") unary
              | "@" unary
              | "sizeof" add
              | primary
primary       = num ("[" expr "]")?  # num / array access
              | "(" expr ")"
              | ident ("(" ( expr "," )* ")" | "[" expr "]")?  # function call / array access
```

## Examples

`while` statement:

```c
int main() {
  int i;
  i = 1;
  while (i < 5)
    i = i + 1;
  return i;
}
```

`for` statement:

```c
int main() {
  int foo;
  int i;
  foo = 0;
  for (i = 0; i < 5; i = i + 1)
    foo = foo + 1;
  return foo;
}
```

`if` statement:

```c
int main() {
  int i;
  i = 0;
  if ((i += 1) == 1)
    return 1;  // executed
  else
    return 0;
}
```

Pointers:

```c
int main() {
  int a;
  int *b;
  a = 0;
  b = &a;
  *b = 3;
  return a;
}
```

Call of external function:

```c
int foo();
int main() { return foo(); }
```

`sizeof` operator:

```c
int main() {
  int a;
  return sizeof(a);  // will return 4
}
```

Also implemented scope!!

```c
int main() {
  int a;
  int b;
  a = 0;
  {
    int a;
    a = 10;
    b = a / 2;
    // return a + b;  # this will return 15
  }
  return a + b;  // will return 5
}
```

## How to build

You can compile this project like this:

```sh
mkdir -p build
cd build
cmake -DCMAKE_BUILD_TYPE=Debug ..
cmake --build .
```

`make` shortcut is also available:

```sh
make
```

## How to test

```sh
./test.sh
```

`make` shortcut is also available:

```sh
make test
```

## Garbage Collection

Ucc implements reference counting garbage collection.

Pointer type variables declared using "`@`" instead of "`*`" (managed pointers) are managed automatically.
Differences between normal pointers and managed pointers are that the region o managed pointers have to be allocated using `rc_malloc()` instead of `malloc()` and you don't need to `free` explicitly.

Note that reference counting garbage collection cannot free circular references.
This means that ucc leaks circular reference structure.

Some examples:

```c
int @rc_malloc(int a);
int main() {
  int @a;
  a = rc_malloc(4);  // (1): Actually allocates additional 4 bytes for counter
  a = rc_malloc(4);  // (2): Same as above, free (1)
  @a = 10;
  return @a;  // free (2)
}
```

```c
int @rc_malloc(int a);
int main() {
  {
    int @a; a = rc_malloc(4);  // (3)
  }  // free (3)
  return 12;
}
```

```c
int @rc_malloc(int a);
int main() {
  int @a;
  a = rc_malloc(4);  // (4)
  {
    return 12;  // free (4)
  }
}
```
