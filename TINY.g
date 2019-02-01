grammar TINY;

options {
    language = C;
}

keyword : 'PROGRAM' | 'BEGIN' | 'END' | 'FUNCTION' | 'READ' | 'WRITE' | 'IF' | 'ELSE' | 'ENDIF' | 'WHILE' | 'ENDWHILE' | 'CONTINUE' | 'BREAK' | 'RETURN' | 'INT' | 'VOID' | 'STRING' | 'FLOAT';

operator : ':=' | '+' | '-' | '*' | '/' | '=' | '!=' | '<' | '>' | '(' | ')' | ';' | ',' | '<=' | '>=';

identifier : ALPHA ALPHANUM*;

INTLITERAL : NUMBER+;
FLOATLITERAL : NUMBER* '.' NUMBER+;
STRINGLITERAL : '"' ~'"'* '"';

COMMENT : '--' ~('\n'|'\r')* '\r'? '\n' { skip(); };

fragment ALPHA : 'a'..'z' | 'A'..'Z';
fragment NUMBER : '0'..'9';
fragment ALPHANUM : NUMBER | ALPHA;

WS : (' '|'\t'|'\r'|'\n')+ { skip(); };
