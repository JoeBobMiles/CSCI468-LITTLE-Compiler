grammar TINY;

/* TODO: needed? */
file: program EOF;

/* Program */
program           : PROGRAM id BEGIN pgmBody END ;
id                : IDENTIFIER ;
pgmBody           : decl funcDeclarations ;
decl              : stringDecl decl | varDecl decl | empty ;

/* Global String Declaration */
stringDecl        : STRING id COLONEQ str SEMICOLON ;
str               : STRINGLITERAL ;

/* Variable Declaration */
varDecl           : varType idList SEMICOLON ;
varType           : FLOAT | INT ;
anyType           : FLOAT | INT | VOID ;
idList            : id idTail ;
idTail            : COMMA id idTail | empty ;

/* Function Paramater List */
paramDeclList     : paramDecl paramDeclTail | empty ;
paramDecl         : varType id ;
paramDeclTail     : COMMA paramDecl paramDeclTail | empty ;

/* Function Declarations */
funcDeclarations  : funcDecl funcDeclarations | empty ;
funcDecl          : FUNCTION anyType id OPENPAREN paramDeclList CLOSEPAREN BEGIN funcBody END ;
funcBody          : decl stmtList ;

/* Statement List */
stmtList          : stmt stmtList | empty ;
stmt              : baseStmt | ifStmt | whileStmt ;
baseStmt          : assignStmt | readStmt | writeStmt | returnStmt ;

/* Basic Statements */
assignStmt        : assignExpr SEMICOLON ;
assignExpr        : id COLONEQ expr ;
readStmt          : READ OPENPAREN idList CLOSEPAREN SEMICOLON ;
writeStmt         : WRITE OPENPAREN idList CLOSEPAREN SEMICOLON ;
returnStmt        : RETURN expr SEMICOLON ;

/* Expressions */
expr              : exprPrefix factor ;
exprPrefix        : exprPrefix factor addop | empty ;
factor            : factorPrefix postfixExpr ;
factorPrefix      : factorPrefix postfixExpr mulop | empty ;
postfixExpr       : primary | callExpr ;
callExpr          : id OPENPAREN exprList CLOSEPAREN ;
exprList          : expr exprListTail | empty ;
exprListTail      : COMMA expr exprListTail | empty ;
primary           : OPENPAREN expr CLOSEPAREN | id | INTLITERAL | FLOATLITERAL ;
addop             : PLUS | MINUS ;
mulop             : STAR | SLASH ;

/* Complex Statements and Condition */
ifStmt            : IF OPENPAREN cond CLOSEPAREN decl stmtList elsePart ENDIF ;
elsePart          : ELSE decl stmtList | empty ;
cond              : expr compop expr ;
compop            : LT | GT | EQUAL | NOTEQ | LTEQ | GTEQ ;

/* While statements */
whileStmt         : WHILE OPENPAREN cond CLOSEPAREN decl stmtList ENDWHILE ;

empty             : ;

PROGRAM           : 'PROGRAM';
BEGIN             : 'BEGIN';
END               : 'END';
FUNCTION          : 'FUNCTION';
READ              : 'READ';
WRITE             : 'WRITE';
IF                : 'IF';
ELSE              : 'ELSE';
ENDIF             : 'ENDIF';
WHILE             : 'WHILE';
ENDWHILE          : 'ENDWHILE';
CONTINUE          : 'CONTINUE';
BREAK             : 'BREAK';
RETURN            : 'RETURN';
INT               : 'INT';
VOID              : 'VOID';
STRING            : 'STRING';
FLOAT             : 'FLOAT';

COLONEQ           : ':=';
PLUS              : '+';
MINUS             : '-';
STAR              : '*';
SLASH             : '/';
EQUAL             : '=';
NOTEQ             : '!=';
LT                : '<';
GT                : '>';
OPENPAREN         : '(';
CLOSEPAREN        : ')';
SEMICOLON         : ';';
COMMA             : ',';
LTEQ              : '<=';
GTEQ              : '>=';

IDENTIFIER        : ALPHA ALPHANUM* ;

INTLITERAL        : NUMBER+ ;
FLOATLITERAL      : NUMBER* '.' NUMBER+ ;
STRINGLITERAL     : '"' .*? ~'\\' '"' ;

// the `.*?` syntax means match any character, but as few as possible.
COMMENT           : '--' .*? '\r'? '\n' -> skip ;

fragment ALPHANUM : NUMBER | ALPHA ;
fragment ALPHA    : [a-zA-Z] ;
fragment NUMBER   : [0-9] ;

WS                : [ \t\r\n]+ -> skip ;
