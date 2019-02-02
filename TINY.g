grammar TINY;

options {
    language = C;
}

keyword: PROGRAM | BEGIN | END | FUNCTION | READ | WRITE | IF | ELSE | ENDIF | WHILE | ENDWHILE | CONTINUE | BREAK | RETURN | INT | VOID | STRING | FLOAT;

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

operator: ASSIGN | PLUS | MINUS | STAR | SLASH | EQUAL | NOTEQUAL | LESSER | GREATER | OPENPAREN | CLOSEPAREN | SEMICOLON | COMMA | LESSEREQUAL | GREATEREQUAL;

ASSIGN:       ':=';
PLUS:         '+';
MINUS:        '-';
STAR:         '*';
SLASH:        '/';
EQUAL:        '=';
NOTEQUAL:     '!=';
LESSER:       '<';
GREATER:      '>';
OPENPAREN:    '(';
CLOSEPAREN:   ')';
SEMICOLON:    ';';
COMMA:        ',';
LESSEREQUAL:  '<=';
GREATEREQUAL: '>=';

identifier: ALPHA ALPHANUM*;

INTLITERAL:    NUMBER+;
FLOATLITERAL:  NUMBER* '.' NUMBER+;
STRINGLITERAL: '"' ~'"'* '"';

COMMENT: '--' ~('\n'|'\r')* '\r'? '\n' { $channel=HIDDEN; };

fragment ALPHA:    'a'..'z' | 'A'..'Z';
fragment NUMBER:   '0'..'9';
fragment ALPHANUM: NUMBER | ALPHA;

WS : (' '|'\t'|'\r'|'\n')+ { $channel=HIDDEN; };
