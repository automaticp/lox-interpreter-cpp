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
