#include "main.h"
#include "printTINY.h"

#include "ast.h"

#include "strings.h"

#include <cstdio>

static inline
u64 writeNewIfBlock(char *dest, u64 size) {
    static u32 count = 0;
    return snprintf(dest, size, "if_%d", ++count);
}

static inline
u64 writeNewWhileBlock(char *dest, u64 size) {
    static u32 count = 0;
    return snprintf(dest, size, "while_%d", ++count);
}

static void printRoot(Program *, AstRoot *, cchar *);

static
void printExpr(AstExpr *expr, s32 altTemp, cchar *jumpLabel) {
    assert(expr);

    cchar *op = 0;
    char opType = expr->logicalType == 'f'? 'r': expr->logicalType;
    s32 tempNumber = altTemp > 0? altTemp: expr->tempNumber;

    switch (expr->type) {
    case EXPR_Addition:           op = "add";  break;
    case EXPR_Subtraction:        op = "sub";  break;
    case EXPR_Multiplication:     op = "mul";  break;
    case EXPR_Division:           op = "div";  break;

    //case EXPR_LessThan:           op = "jlt";  break;
    //case EXPR_GreaterThan:        op = "jgt";  break;
    //case EXPR_Equal:              op = "jeq"; break;
    //case EXPR_NotEqual:           op = "jne"; break;
    //case EXPR_LessThanOrEqual:    op = "jge"; break;
    //case EXPR_GreaterThanOrEqual: op = "jle"; break;

    /* NOTE: we use the inverse op here */
    case EXPR_LessThan:           op = "jge";  break;
    case EXPR_GreaterThan:        op = "jle";  break;
    case EXPR_Equal:              op = "jne"; break;
    case EXPR_NotEqual:           op = "jeq"; break;
    case EXPR_LessThanOrEqual:    op = "jgt"; break;
    case EXPR_GreaterThanOrEqual: op = "jlt"; break;

    default: break;
    }

    switch (expr->type) {
    case EXPR_Addition:
    case EXPR_Subtraction:
    case EXPR_Multiplication:
    case EXPR_Division: {
        assert(op);

        AstExpr *lhs = expr->asBinaryOp.leftChild;
        AstExpr *rhs = expr->asBinaryOp.rightChild;

        printExpr(lhs, tempNumber, 0);

        switch (rhs->type) {
        case EXPR_IntLiteral:
        case EXPR_FloatLiteral:
        case EXPR_Symbol:
            printf("%s%c %s r%d\n", op, opType,
                   rhs->asTerminal.text, tempNumber);
            break;

        default:
            printExpr(rhs, -1, 0);
            printf("%s%c r%d r%d\n", op, opType,
                   rhs->tempNumber, tempNumber);
            break;
        }
    } break;

    case EXPR_LessThan:
    case EXPR_GreaterThan:
    case EXPR_Equal:
    case EXPR_NotEqual:
    case EXPR_LessThanOrEqual:
    case EXPR_GreaterThanOrEqual: {
        assert(op);
        assert(jumpLabel);

        AstExpr *lhs = expr->asBinaryOp.leftChild;
        AstExpr *rhs = expr->asBinaryOp.rightChild;

        printExpr(rhs, -1, 0);

        switch (rhs->type) {
        case EXPR_IntLiteral:
        case EXPR_FloatLiteral:
        case EXPR_Symbol:
            printf("cmp%c %s r%d\n", opType,
                lhs->asTerminal.text, rhs->tempNumber);
            break;

        default:
            printExpr(lhs, -1, 0);
            printf("cmp%c r%d r%d\n", opType,
                lhs->tempNumber, rhs->tempNumber);
            break;
        }

        printf("%s %s\n", op, jumpLabel);
    } break;

    case EXPR_IntLiteral:
    case EXPR_FloatLiteral:
    case EXPR_Symbol:
        printf("move %s r%d\n", expr->asTerminal.text, tempNumber);
        break;

    case EXPR_Function:
        /* TODO: this */
        printf("jsr <Function %s>\n", expr->asFuncCall.functionName);
        break;

    InvalidDefaultCase;
    }
};

static
void printStatement(Program *program, AstStatement *statement) {
    switch (statement->type) {
    case STATEMENT_Root: {
        char buffer[128] = {};
        switch (statement->asRoot->type) {
        case ROOT_If:
            writeNewIfBlock(buffer, sizeof buffer);
            break;

        case ROOT_While:
            writeNewWhileBlock(buffer, sizeof buffer);
            break;

        InvalidDefaultCase;
        }

        printf("\n");
        printRoot(program, statement->asRoot, buffer);
        printf("\n");
    } break;

    case STATEMENT_Assign: {
        AstExpr *rhs = statement->asAssign.expr;

        switch (rhs->type) {
        case EXPR_IntLiteral:
        case EXPR_FloatLiteral:
            printf("move %s %s\n",
                   rhs->asTerminal.text, statement->asAssign.symbol);
            break;

        default:
            printExpr(rhs, -1, 0);
            printf("move r%d %s\n",
                   rhs->tempNumber, statement->asAssign.symbol);
        }
    } break;

    case STATEMENT_Read: {
        AstExpr *expr = statement->asRead;
        while (expr) {
            cchar *syscall = 0;

            assert(expr->type == EXPR_Symbol);
            SymbolEntry *symbol = findDecl(program, expr->asTerminal.text);
            assert(symbol); /* TODO: don't assert, check! */

            switch (symbol->logicalType) {
            case 'i': syscall = "readi"; break;
            case 'f': syscall = "readr"; break;
            InvalidDefaultCase;
            }

            assert(syscall);
            printf("sys %s %s\n", syscall, expr->asTerminal.text);

            expr = expr->asTerminal.nextParam;
        }
    } break;

    case STATEMENT_Write: {
        AstExpr *expr = statement->asRead;
        while (expr) {
            cchar *syscall = 0;

            assert(expr->type == EXPR_Symbol);
            SymbolEntry *symbol = findDecl(program, expr->asTerminal.text);
            assert(symbol); /* TODO: don't assert, check! */

            switch (symbol->logicalType) {
            case 'i': syscall = "writei"; break;
            case 'f': syscall = "writer"; break;
            case 's': syscall = "writes"; break;
            InvalidDefaultCase;
            }

            assert(syscall);
            printf("sys %s %s\n", syscall, expr->asTerminal.text);

            expr = expr->asTerminal.nextParam;
        }
    } break;

    case STATEMENT_Return:
        printExpr(statement->asReturn, -1, 0);
        printf("<TODO: return r%d>", statement->asReturn->tempNumber);
        break;

    InvalidDefaultCase;
    }
}

static
void printRoot(Program *program, AstRoot *root, cchar *name) {
    assert(program);
    assert(root);
    assert(name);

    SymbolTable *scope = root->symbols;
    openScope(program, scope);

    /*
     * Open block
     */

    switch (root->type) {
    case ROOT_Global:
        break;

    case ROOT_Function:
        printf("\nlabel %s\n", name);
        break;

    case ROOT_If: {
        AstIfRoot *ifRoot = (AstIfRoot *)root;

        AstExpr *comparison = ifRoot->comparison;

        char buffer[128];
        if (ifRoot->elsePart) {
            snprintf(buffer, sizeof buffer, "%s_else", name);
        }
        else {
            snprintf(buffer, sizeof buffer, "%s_end", name);
        }

        printExpr(comparison, -1, buffer);

        printf("label %s\n", name);
    } break;

    case ROOT_While: {
        AstWhileRoot *whileRoot = (AstWhileRoot *)root;

        AstExpr *comparison = whileRoot->comparison;

        char buffer[128];
        snprintf(buffer, sizeof buffer, "%s_end", name);

        printf("label %s\n", name);
        printExpr(comparison, -1, buffer);
        printf("\n");

    } break;

    case ROOT_Else:
        printf("label %s_else\n", name);
        break;

    InvalidDefaultCase;
    }

    /*
     * Internal block
     */

    /* TODO: move variables into printProgramAsAsm(), as there are no local
     * variables in TINY asm. Also, figure out a way to name them for global
     * uniqueness. */
    printf("; Variables\n");
    for (u32 i = 0; i < scope->count; ++i) {
        u32 index = scope->order[i];
        SymbolEntry entry = scope->data[index];

        if (entry.symbolType == 'v') {
            if (entry.value) {
                printf("str %s %s\n", entry.id, entry.value);
            }
            else {
                printf("var %s\n", entry.id);
            }
        }
    }

    if (root->type == ROOT_Global) {
        /* print out main() first, because I think the pseudo-asm goes
         * top-to-bottom, and we want to hit main() first */
        SymbolEntry *mainEntry = findDecl(program, "main");
        assert(mainEntry); /* TODO: don't assert, check! */
        if (mainEntry && mainEntry->symbolType == 'f') {
            printRoot(program, mainEntry->root, "main");
        }

        printf("\n; Other Functions\n\n");
        for (u32 i = 0; i < scope->count; ++i) {
            u32 index = scope->order[i];
            SymbolEntry *entry = scope->data + index;

            if (entry->symbolType == 'f') {
                /* ...but skip main() */
                if (!stringsAreEqual(entry->id, "main")) {
                    cchar *name = entry->root->symbols->name;
                    printRoot(program, entry->root, name);
                }
            }
        }
    }
    else {
        printf("\n; Statements\n\n");

        AstStatement *statement = root->firstStatement;
        while (statement) {
            printStatement(program, statement);

            statement = statement->nextStatement;
        }
    }

    /*
     * Close block
     */

    switch (root->type) {
    case ROOT_Global:
        break;

    case ROOT_Function:
        if (stringsAreEqual(name, "main")) {
            printf("\nend\n");
        }
        else {
            printf("\nret\n");
        }
        break;

    case ROOT_Else:
        printf("label %s_end\n", name);
        break;

    case ROOT_If: {
        AstRoot *elseRoot = ((AstIfRoot *)root)->elsePart;
        if (elseRoot) {
            printf("jmp %s_end\n", name);
            printRoot(program, elseRoot, name);
        }
        else {
            printf("label %s_end\n", name);
        }
    } break;

    case ROOT_While:
        printf("jmp %s\n", name);
        printf("label %s_end\n", name);
        break;

    InvalidDefaultCase;
    }

    closeScope(program);
}

void printProgramAsAsm(Program *program) {
    printRoot(program, &program->root, "GLOBAL");
}
