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

|Type|Order|Associativity|Form|
|---|---|---|---|
|literal|0|-|*literal*|
|group|0|-|(*expr*)|
|unary|1|RA|( + \| - \| ! )*expr*|
|factor|2|LA|*expr* ( \* \| / ) *expr*|
|term|3|LA|*expr* ( + \| - ) *expr*|
|comp|4|LA|*expr* ( > \| >= \| < \| <= ) *expr*|
|equal|5|LA| *expr* ( == \| != ) *expr*|
|ternary|6|RA (values)| *expr* ? *expr* : *expr*|
|comma|7|LA| *expr*, *expr*|



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
