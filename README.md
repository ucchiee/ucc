# ucc

Ucc is a (subset of) C compiler, now able to handle this grammar:

```
program       = funcdef*
funcdef       = ident "(" param_decl ("," param_decl)* ")" compound_stmt
param_decl    = ident
stmt          = expr ";"
              | compound_stmt
              | "if" "(" expr ")" stmt ("else" stmt)?
              | "while" "(" expr ")" stmt
              | "for" "(" expr? ";" expr? ";" expr? ")" stmt
              | "return" expr ";"
compound_stmt = "{" stmt* "}"
expr          = assign
assign        = equality ("=" assign | "+=" assign)?
equality      = relational ("==" relational | "!=" relational)*
relational    = add ("<" add | "<=" add | ">" add | ">=" add)*
add           = mul ("+" mul | "-" mul)*
mul           = unary ("*" unary | "/" unary)*
unary         = ("+" | "-")? primary
              | ("*" | "&")? unary
primary       = num
              | "(" expr ")"
              | ident ("(" ( expr "," )* ")")?  # function call
```
