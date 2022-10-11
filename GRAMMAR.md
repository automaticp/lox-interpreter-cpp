# Simple Expression Grammar

EBNF:

```EBNF
expression = binary_expression | unary_expression | parenthesized_expression | literal;


binary_expression = expression, binary_op, expression;

binary_op = "+" | "-" | "*" | "/" | "==" | "!=" | "<=" | ">=" | "<" | ">";


unary_expression = unary_op, expression;

unary_op = "!" | "-" | "+";


parenthesized_expression = "(", expression, ")";


literal = number | string | boolean | "nil";

boolean = "true" | "false";

number = digit, { digit }, [".", digit, { digit }];

digit = "0" | "1" | "2" | "3" | "4" | "5" | "6" | "7" | "8" | "9";

string = '"', { character }, '"';

character = ? any character or escape sequence ?;

```
