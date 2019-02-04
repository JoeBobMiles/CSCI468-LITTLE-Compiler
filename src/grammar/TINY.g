grammar TINY;

// ANTLR expects 'rules' in the grammar to be lowercase.
keyword : 'PROGRAM' | 'BEGIN' | 'END' | 'FUNCTION' | 'READ' | 'WRITE' | 'IF' | 'ELSE' | 'ENDIF' | 'WHILE' | 'ENDWHILE' | 'CONTINUE' | 'BREAK' | 'RETURN' | 'INT' | 'VOID' | 'STRING' | 'FLOAT' ;

opr : ':=' | '+' | '-' | '*' | '/' | '=' | '!=' | '<' | '>' | '(' | ')' | ';' | ',' | '<=' | '>=' ;

identifier : (ALPHA) (ALPHANUM)* ;

literal : INTLITERAL | FLOATLITERAL | STRINGLITERAL ;

INTLITERAL : NUMBER+ ;
FLOATLITERAL : NUMBER* '.' NUMBER+ ;
STRINGLITERAL : '"' ~'"'* '"' ;

comment : '--' ~('\n'|'\r')* '\r'? '\n' ;

ALPHANUM : NUMBER|ALPHA ;
ALPHA : ('a'..'z'|'A'..'Z') ;
NUMBER : '0'..'9' ;

WS : [ \t\r\n]+ -> skip ;
