#include "main.h"
#include "printTINY.h"

#include "ast.h"

#include "strings.h"

#include <cstdio>

static void printRoot(Program *, AstRoot *);

static
void printExpr(AstExpr *expr) {
    assert(expr);

    cchar *op = 0;

    switch (expr->type) {
    case EXPR_Addition:           op = "+";  break;
    case EXPR_Subtraction:        op = "-";  break;
    case EXPR_Multiplication:     op = "*";  break;
    case EXPR_Division:           op = "/";  break;
    case EXPR_LessThan:           op = "<";  break;
    case EXPR_GreaterThan:        op = ">";  break;
    case EXPR_Equal:              op = "="; break;
    case EXPR_NotEqual:           op = "!="; break;
    case EXPR_LessThanOrEqual:    op = "<="; break;
    case EXPR_GreaterThanOrEqual: op = ">="; break;
    default: break;
    }

    switch (expr->type) {
    case EXPR_Addition:
    case EXPR_Subtraction:
    case EXPR_Multiplication:
    case EXPR_Division:
    case EXPR_LessThan:
    case EXPR_GreaterThan:
    case EXPR_Equal:
    case EXPR_NotEqual:
    case EXPR_LessThanOrEqual:
    case EXPR_GreaterThanOrEqual:
        assert(op);
        //printf("(%d: ", expr->tempNumber);
        printf("(");
        printExpr(expr->asBinaryOp.leftChild);
        printf(" %s ", op);
        printExpr(expr->asBinaryOp.rightChild);
        printf(")");
        break;

    case EXPR_IntLiteral:
    case EXPR_FloatLiteral:
    case EXPR_StringLiteral:
    case EXPR_Symbol:
        printf("%s", expr->asTerminal.text);
        break;

    case EXPR_Function:
        /* TODO: this */
        printf("<Function %s>", expr->asFuncCall.functionName);
        break;

    case EXPR_Null: InvalidCodePath;
    }
};

static
void printStatement(Program *program, AstStatement *statement, cchar *indent) {
    switch (statement->type) {
    case STATEMENT_Root:
        printRoot(program, statement->asRoot);
        break;

    case STATEMENT_Assign:
        printf("%s%s := ", indent, statement->asAssign.symbol);
        printExpr(statement->asAssign.expr);
        printf(";\n");
        break;

    case STATEMENT_Read: {
        printf("%sREAD(", indent);

        cchar *prefix = "";
        AstExpr *expr = statement->asRead;
        while (expr) {
            printf(prefix);
            printExpr(expr);
            prefix = ", ";
            expr = expr->asTerminal.nextParam;
        }

        printf(");\n");
        break;
    }

    case STATEMENT_Write: {
        printf("%sWRITE(", indent);

        cchar *prefix = "";
        AstExpr *expr = statement->asRead;
        while (expr) {
            printf(prefix);
            printExpr(expr);
            prefix = ", ";
            expr = expr->asTerminal.nextParam;
        }

        printf(");\n");
    } break;

    case STATEMENT_Return:
        printf("%sreturn ", indent);
        printExpr(statement->asReturn);
        printf(";\n");
        break;

    InvalidDefaultCase;
    }
}

static
void printRoot(Program *program, AstRoot *root) {
    /* 64 levels of indent should be enough for anyone */
    static char indent[64]; /* TODO: de-static-ify this? */
    static char *indentCur = indent;

    assert(program);
    assert(root);
    assert(indentCur >= indent);
    *indentCur = 0; /* cap the current indent amount */

    SymbolTable *scope = root->symbols;
    openScope(program, scope);

    switch (root->type) {
    case ROOT_Null:
        printf("%s<NULL ROOT>\n", indent);
        break;

    case ROOT_Global:
        printf("%sPROGRAM root\n%sBEGIN", indent, indent);
        break;

    case ROOT_Else:
        printf("%sELSE\n", indent);
        break;

    case ROOT_Function: {
        cchar *prefix = "";
        cchar *funcName = scope->name;

        SymbolEntry *symbol = findDecl(program, funcName);
        assert(symbol); /* TODO: don't assert, check! */

        cchar *type = typeString(symbol->logicalType);

        printf("%sFUNCTION %s %s(", indent, type, funcName);
        for (u32 i = 0; i < scope->count; ++i) {
            u32 index = scope->order[i];
            SymbolEntry entry = scope->data[index];

            if (entry.symbolType == 'p') {
                cchar *type = typeString(entry.logicalType);
                printf("%s%s %s", prefix, type, entry.id);
                prefix = ", ";
            }
        }
        printf(")\n%sBEGIN\n", indent);
    } break;

    case ROOT_If: {
        AstIfRoot *ifRoot = (AstIfRoot *)root;
        printf("%sIF ", indent);
        printExpr(ifRoot->comparison);
        printf("\n");
    } break;

    case ROOT_While: {
        AstWhileRoot *whileRoot = (AstWhileRoot *)root;
        printf("%sWHILE ", indent);
        printExpr(whileRoot->comparison);
        printf("\n");
    } break;
    }

    *indentCur++ = '\t';

    printf("%s-- Variables:\n\n", indent);
    for (u32 i = 0; i < scope->count; ++i) {
        u32 index = scope->order[i];
        SymbolEntry entry = scope->data[index];

        if (entry.symbolType == 'v') {
            cchar *type = typeString(entry.logicalType);

            printf("%s%s %s", indent, type, entry.id);
            if (entry.value) {
                printf(" := %s", entry.value);
            }
            printf(";\n");
        }
    }

    if (root->type == ROOT_Global) {
        /* print out main() first, because I think the pseudo-asm goes
         * top-to-bottom, and we want to hit main() first */
        printf("\n%s-- Main:\n\n", indent);
        SymbolEntry *mainEntry = getSymbol(scope, "main");
        if (mainEntry && mainEntry->symbolType == 'f') {
            printRoot(program, mainEntry->root);
        }

        printf("\n%s-- Other Functions:\n\n", indent);
        for (u32 i = 0; i < scope->count; ++i) {
            u32 index = scope->order[i];
            SymbolEntry *entry = scope->data + index;

            /* ...but skip main() */
            if (entry->symbolType == 'f' && !stringsAreEqual(entry->id, "main")) {
                printRoot(program, entry->root);
            }
        }
    }
    else {
        printf("\n%s-- Statements:\n\n", indent);

        AstStatement *statement = root->firstStatement;
        while (statement) {
            printStatement(program, statement, indent);

            statement = statement->nextStatement;
        }
    }

    *--indentCur = '\0';

    switch (root->type) {
    default:
        printf("%sEND\n", indent);
        break;

    case ROOT_Else:
        printf("%sENDIF\n", indent);
        break;

    case ROOT_If: {
        AstRoot *elseRoot = ((AstIfRoot *)root)->elsePart;
        if (elseRoot) {
            printRoot(program, elseRoot);
        }
        else {
            printf("%sENDIF\n", indent);
        }
    } break;

    case ROOT_While:
        printf("%sENDWHILE\n", indent);
        break;
    }

    closeScope(program);
}

void printProgramAsTINY(Program *program) {
    printRoot(program, &program->root);
}
