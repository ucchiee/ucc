# ucc

Ucc is a (subset of) C compiler, now able to handle this grammar:

```
program    = stmt*
stmt       = expr ";"
           | "{" stmt* "}"
           | "if" "(" expr ")" stmt ("else" stmt)?
           | "while" "(" expr ")" stmt
           | "for" "(" expr? ";" expr? ";" expr? ")" stmt
           | "return" expr ";"
expr       = assign
assign     = equality ("=" assign | "+=" assign)?
equality   = relational ("==" relational | "!=" relational)*
relational = add ("<" add | "<=" add | ">" add | ">=" add)*
add        = mul ("+" mul | "-" mul)*
mul        = unary ("*" unary | "/" unary)*
unary      = ("+" | "-")? primary
           | ("*" | "&")? unary
primary    = num
           | "(" expr ")"
           | ident ("(" ")")?  # function call
```
