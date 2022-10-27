# Simple Expression Grammar

```EBNF
expression = binary_expr | unary_expr | parenthesized_expr | literal;


binary_expr = expression, binary_op, expression;

binary_op = "+" | "-" | "*" | "/" | "==" | "!=" | "<=" | ">=" | "<" | ">";


unary_expr = unary_op, expression;

unary_op = "!" | "-" | "+";


parenthesized_expr = "(", expression, ")";


literal = number | string | boolean | "nil";

boolean = "true" | "false";

number = digit, { digit }, [".", digit, { digit }];

digit = "0" | "1" | "2" | "3" | "4" | "5" | "6" | "7" | "8" | "9";

string = '"', { character }, '"';

character = ? any character or escape sequence ?;

```

# Expression Grammar With Precedence and Associativity

```EBNF

primary_expr = literal | parenthesized_expr;
(*
literal = ...
...
*)
parenthesized_expr = "(", expression, ")";


unary_expr = unary_op, unary_expr | primary_expr;
unary_op = "!" | "-" | "+";

(*
factor_expr = factor_expr, factor_op, unary_expr | unary_expr;
*)
factor_expr = unary_expr, { factor_op, unary_expr };
factor_op = "/" | "*";


term_expr = factor_expr, { term_op, factor_expr };
term_op = "+" | "-";


comparison_expr = term_expr, { comparison_op, term_expr };
comparison_op = "<=" | ">=" | "<" | ">";


equality_expr = comparison_expr, { equality_op, comparison_expr };
equality_op = "==" | "!=";


expression = eqiality_expr;
```

## Extras:

Ternary (Right-Associative for values):
```EBNF
ternary_expr = condition, "?", ternary_expr, ":", ternary_expr | equality_expr;
condition = equality_expr; (* must be convertible to bool *)

expression = ternary_expr;
```

Comma operator (Lowest precedence, Left-Associative):
```EBNF
comma_expr = ternary_expr, { ",", ternary_expr };

expression = comma_expr;
```


Error productions:
```EBNF
error_binary_without_left_operand = binary_op, expression;
```


# Operator Precedence Table

*Higher order = lower precedence*

*N/I = Not Implemented*

|Type|Order|Associativity|Form|
|---|---|---|---|
|literal|0|-|*literal*|
|group|0|-|(*expr*)|
|call|1|LA|*expr*([ *expr*, ... ])|
|unary|2|RA|( + \| - \| ! )*expr*|
|factor|3|LA|*expr* ( \* \| / ) *expr*|
|term|4|LA|*expr* ( + \| - ) *expr*|
|comp|5|LA|*expr* ( > \| >= \| < \| <= ) *expr*|
|equal|6|LA| *expr* ( == \| != ) *expr*|
|and|7|LA|*expr* and *expr*|
|or|8|LA|*expr* or *expr*|
|ternary (N/I)|9|RA (values)| *expr* ? *expr* : *expr*|
|assign|10|RA| *lvalue* = *expr*|
|comma (N/I)|11|LA| *expr*, *expr*|



# Script Grammar

```EBNF
program = { statement }, ?EOF?;

statememt = expression_stmt | print_stmt;

expression_stmt = expression, ";";

print_stmt = "print", expression, ";";

```

## Variable declaration

```EBNF
program = { declaration }, ?EOF?;

declaration = statement | var_decl;


var_decl = "var", identifier, ["=", expression], ";";

identifier = alpha_char, { alnum_char };
alpha_char = ?letter? | "_";
alnum_char = alpha_char | digit;

primary_expr = literal | parenthesized_expr | identifier;

```


## Assignment expression

```EBNF
assignment_expr = identifier, "=", assignment_expr | equality_expr;

expression = assignment_expr;
```

## Block statement

```ENBF
statement = expression_stmt | print_stmt | block;

block = "{", { declaration }, "}";
```



# Control Flow

## If statement

```EBNF
statement = expression_stmt | if_stmt | print_stmt | block;

if_stmt = "if", "(", expression, ")", statement, [ "else", statement ];
```


## Short-Circuiting operators

```EBNF
assignment_expr = identifier, "=", assignment_expr | or_expr;

or_expr = and_expr, { "or", and_expr };
and_expr = equality_expr, { "and", equality_expr };
```


## While loops

```EBNF
statement = expression_stmt | if_stmt | while_stmt | print_stmt | block;

while_stmt = "while", "(", expression, ")", statement;
```


## For loops

```EBNF
statement = expression_stmt | if_stmt | while_stmt | for_stmt | print_stmt | block;

for_stmt = "for", "("
    ( var_decl | expression_stmt | ";" ),
    [ expression ], ";",
    [ expression ],
")", statement;
```


# Functions

## Call expression

```EBNF
unary_expr = unary_op, unary | call_expr;

call_expr = primary_expr, { "(", [ arguments ], ")" };

arguments = expression, { ",", expression };
```


## Function declaration

```EBNF
declaration = fun_decl | var_decl | statement;

fun_decl = "fun", identifier, "(", [ parameters ], ")", block;

parameters = identifier, { ",", identifier };
```
