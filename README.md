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
