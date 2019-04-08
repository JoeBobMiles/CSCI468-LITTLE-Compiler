#include "main.h"
#include "ast.h"

#include "strings.h"

AstStatement *makeStatement(StatementType type) {
    AstStatement *result = (AstStatement *)malloc(sizeof *result);
    zeroMemory((char *)result, sizeof *result);

    result->type = type;

    return result;
}
