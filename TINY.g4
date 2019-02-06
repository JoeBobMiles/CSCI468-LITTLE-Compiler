grammar TINY;

// TODO: actual file structure
//file : expr_list EOF ;

//expr_list : (expr SEMICOLON)+ ;

// TODO: more expressions
//expr : IDENTIFIER COLONEQ literal ;

// probably unneccesary
KEYWORD : PROGRAM | BEGIN | END | FUNCTION | READ | WRITE | IF | ELSE | ENDIF | WHILE | ENDWHILE | CONTINUE | BREAK | RETURN | INT | VOID | STRING | FLOAT ;

fragment PROGRAM:  'PROGRAM';
fragment BEGIN:    'BEGIN';
fragment END:      'END';
fragment FUNCTION: 'FUNCTION';
fragment READ:     'READ';
fragment WRITE:    'WRITE';
fragment IF:       'IF';
fragment ELSE:     'ELSE';
fragment ENDIF:    'ENDIF';
fragment WHILE:    'WHILE';
fragment ENDWHILE: 'ENDWHILE';
fragment CONTINUE: 'CONTINUE';
fragment BREAK:    'BREAK';
fragment RETURN:   'RETURN';
fragment INT:      'INT';
fragment VOID:     'VOID';
fragment STRING:   'STRING';
fragment FLOAT:    'FLOAT';

// probably unneccesary
OPERATOR : COLONEQ | PLUS | MINUS | STAR | SLASH | EQUAL | NOTEQ | LT | GT | OPENPAREN | CLOSEPAREN | SEMICOLON | COMMA | LTEQ | GTEQ ;

fragment COLONEQ:    ':=';
fragment PLUS:       '+';
fragment MINUS:      '-';
fragment STAR:       '*';
fragment SLASH:      '/';
fragment EQUAL:      '=';
fragment NOTEQ:      '!=';
fragment LT:         '<';
fragment GT:         '>';
fragment OPENPAREN:  '(';
fragment CLOSEPAREN: ')';
fragment SEMICOLON:  ';';
fragment COMMA:      ',';
fragment LTEQ:       '<=';
fragment GTEQ:       '>=';

IDENTIFIER : ALPHA ALPHANUM* ;

literal : INTLITERAL | FLOATLITERAL | STRINGLITERAL ;

INTLITERAL : NUMBER+ ;
FLOATLITERAL : NUMBER* '.' NUMBER+ ;
STRINGLITERAL : '"' .*? ~'\\' '"' ;

// NOTE: the `.*?` syntax means match any character, but as few as possible.
COMMENT : '--' .*? '\r'? '\n' -> skip ;

fragment ALPHANUM: NUMBER | ALPHA ;
fragment ALPHA:    [a-zA-Z] ;
fragment NUMBER:   [0-9] ;

WS : [ \t\r\n]+ -> skip ;
