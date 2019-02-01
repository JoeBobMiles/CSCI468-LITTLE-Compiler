grammar TINY;

options {
    language = C;
}

WS : (' '|'\t'|'\r'|'\n')+ { skip(); };

KEYWORD : 'PROGRAM' | 'BEGIN' | 'END' | 'FUNCTION' | 'READ' | 'WRITE' | 'IF' | 'ELSE' | 'ENDIF' | 'WHILE' | 'ENDWHILE' | 'CONTINUE' | 'BREAK' | 'RETURN' | 'INT' | 'VOID' | 'STRING' | 'FLOAT';

OPERATOR : ':=' | '+' | '-' | '*' | '/' | '=' | '!=' | '<' | '>' | '(' | ')' | ';' | ',' | '<=' | '>=';

IDENTIFIER : (ALPHA) (ALPHANUM)*;

LITERAL : INTLITERAL | FLOATLITERAL | STRINGLITERAL;

INTLITERAL : NUMBER+;
FLOATLITERAL : NUMBER* '.' NUMBER+;
STRINGLITERAL : '"' ~'"'* '"';

COMMENT : '--' ~('\n'|'\r'|)* '\r'? '\n';

ALPHANUM : NUMBER|ALPHA;
ALPHA : ('a'..'z'|'A'..'Z');
NUMBER : '0'..'9';

