#include "main.h"
#include "ast.h"

AstStatement *makeStatement(StatementType type) {
    AstStatement *result = 0;

    switch (type) {
    case STATEMENT_Root:
        result = (AstStatement *)malloc(sizeof (AstRootStatement));
        *(AstRootStatement *)result = (AstRootStatement){
            .header = { },
            .root = 0,
        };
        break;

    case STATEMENT_Assign:
        result = (AstStatement *)malloc(sizeof (AstAssignStatement));
        *(AstAssignStatement *)result = (AstAssignStatement){
            .header = { },
            .symbol = 0,
            .expr = 0,
        };
        break;

    case STATEMENT_Read:
        /* TODO */
        result = (AstStatement *)malloc(sizeof (AstStatement));
        break;

    case STATEMENT_Write:
        /* TODO */
        result = (AstStatement *)malloc(sizeof (AstStatement));
        break;

    case STATEMENT_Return:
        result = (AstStatement *)malloc(sizeof (AstReturnStatement));
        *(AstReturnStatement *)result = (AstReturnStatement){
            .header = { },
            .expr = 0,
        };
        break;

    default: InvalidCodePath;
    };

    assert(result);

    result->nextStatement = 0;
    result->type = type;

    return result;
}
