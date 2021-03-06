# ucc

Ucc is a (subset of) C compiler, now able to handle this grammar:

```
program       = funcdef*
type_specifier= "int"
declarator    = ident | "*" declarator
funcdef_decl  = param_decl "(" param_decl ("," param_decl)* ")" (compound_stmt | ";")
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
              | primary
primary       = num
              | "(" expr ")"
              | ident ("(" ( expr "," )* ")")?  # function call
```
