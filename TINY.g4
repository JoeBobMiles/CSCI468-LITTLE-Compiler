grammar TINY;

/* TODO: needed? */
file: program EOF;

/* Program */
program           : PROGRAM id BEGIN pgm_body END ;
id                : IDENTIFIER ;
pgm_body          : decl func_declarations ;
decl              : string_decl decl | var_decl decl | empty ;

/* Global String Declaration */
string_decl       : STRING id COLONEQ str SEMICOLON ;
str               : STRINGLITERAL ;

/* Variable Declaration */
var_decl          : var_type id_list SEMICOLON ;
var_type          : FLOAT | INT ;
any_type          : FLOAT | INT | VOID ;
id_list           : id id_tail ;
id_tail           : COMMA id id_tail | empty ;

/* Function Paramater List */
param_decl_list   : param_decl param_decl_tail | empty ;
param_decl        : var_type id ;
param_decl_tail   : COMMA param_decl param_decl_tail | empty ;

/* Function Declarations */
func_declarations : func_decl func_declarations | empty ;
func_decl         : FUNCTION any_type id OPENPAREN param_decl_list CLOSEPAREN BEGIN func_body END ;
func_body         : decl stmt_list ;

/* Statement List */
stmt_list         : stmt stmt_list | empty ;
stmt              : base_stmt | if_stmt | while_stmt ;
base_stmt         : assign_stmt | read_stmt | write_stmt | return_stmt ;

/* Basic Statements */
assign_stmt       : assign_expr SEMICOLON ;
assign_expr       : id COLONEQ expr ;
read_stmt         : READ OPENPAREN id_list CLOSEPAREN SEMICOLON ;
write_stmt        : WRITE OPENPAREN id_list CLOSEPAREN SEMICOLON ;
return_stmt       : RETURN expr SEMICOLON ;

/* Expressions */
expr              : expr_prefix factor ;
expr_prefix       : expr_prefix factor addop | empty ;
factor            : factor_prefix postfix_expr ;
factor_prefix     : factor_prefix postfix_expr mulop | empty ;
postfix_expr      : primary | call_expr ;
call_expr         : id OPENPAREN expr_list CLOSEPAREN ;
expr_list         : expr expr_list_tail | empty ;
expr_list_tail    : COMMA expr expr_list_tail | empty ;
primary           : OPENPAREN expr CLOSEPAREN | id | INTLITERAL | FLOATLITERAL ;
addop             : PLUS | MINUS ;
mulop             : STAR | SLASH ;

/* Complex Statements and Condition */
if_stmt           : IF OPENPAREN cond CLOSEPAREN decl stmt_list else_part ENDIF ;
else_part         : ELSE decl stmt_list | empty ;
cond              : expr compop expr ;
compop            : LT | GT | EQUAL | NOTEQ | LTEQ | GTEQ ;

/* While statements */
while_stmt        : WHILE OPENPAREN cond CLOSEPAREN decl stmt_list ENDWHILE ;

empty:;

PROGRAM:  'PROGRAM';
BEGIN:    'BEGIN';
END:      'END';
FUNCTION: 'FUNCTION';
READ:     'READ';
WRITE:    'WRITE';
IF:       'IF';
ELSE:     'ELSE';
ENDIF:    'ENDIF';
WHILE:    'WHILE';
ENDWHILE: 'ENDWHILE';
CONTINUE: 'CONTINUE';
BREAK:    'BREAK';
RETURN:   'RETURN';
INT:      'INT';
VOID:     'VOID';
STRING:   'STRING';
FLOAT:    'FLOAT';

COLONEQ:    ':=';
PLUS:       '+';
MINUS:      '-';
STAR:       '*';
SLASH:      '/';
EQUAL:      '=';
NOTEQ:      '!=';
LT:         '<';
GT:         '>';
OPENPAREN:  '(';
CLOSEPAREN: ')';
SEMICOLON:  ';';
COMMA:      ',';
LTEQ:       '<=';
GTEQ:       '>=';

IDENTIFIER : ALPHA ALPHANUM* ;

INTLITERAL : NUMBER+ ;
FLOATLITERAL : NUMBER* '.' NUMBER+ ;
STRINGLITERAL : '"' .*? ~'\\' '"' ;

// NOTE: the `.*?` syntax means match any character, but as few as possible.
COMMENT : '--' .*? '\r'? '\n' -> skip ;

fragment ALPHANUM: NUMBER | ALPHA ;
fragment ALPHA:    [a-zA-Z] ;
fragment NUMBER:   [0-9] ;

WS : [ \t\r\n]+ -> skip ;
