grammar TINY;

// ANTLR expects 'rules' in the grammar to be lowercase.
keyword : PROGRAM | BEGIN | END | FUNCTION | READ | WRITE | IF | ELSE | ENDIF | WHILE | ENDWHILE | CONTINUE | BREAK | RETURN | INT | VOID | STRING | FLOAT ;

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

opr : COLONEQ | PLUS | MINUS | STAR | SLASH | EQUAL | NOTEQ | LT | GT | OPENPAREN | CLOSEPAREN | SEMICOLON | COMMA | LTEQ | GTEQ ;

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

identifier : (ALPHA) (ALPHANUM)* ;

literal : INTLITERAL | FLOATLITERAL | STRINGLITERAL ;

INTLITERAL : NUMBER+ ;
FLOATLITERAL : NUMBER* '.' NUMBER+ ;
STRINGLITERAL : '"' .*? '"' ;

comment : '--' .*? '\r'? '\n' ;

ALPHANUM : NUMBER|ALPHA ;
ALPHA : [a-zA-Z] ;
NUMBER : [0-9] ;

WS : [ \t\r\n]+ -> skip ;
