#include "main.h"
#include "ast.h"

#include "strings.h"
#include "symbol-table.h"

#include <antlr4-runtime.h>
#include <TINYLexer.h>
#include <TINYParser.h>

#include "simplify-antlr-types.h"

using namespace antlr4;
using namespace antlr4::tree;
using namespace std;

AstStatement *makeStatement(StatementType type) {
    AstStatement *result = (AstStatement *)malloc(sizeof *result);
    zeroMemory((char *)result, sizeof *result);

    result->type = type;

    return result;
}

SymbolTable *openNewScope(Program *program, cchar *scopeName) {
    assert(program->listCount < MAX_TABLES);

    SymbolTable *table = program->tableList + program->listCount++;

    initSymbolTable(table, saveString(scopeName), 16);

    program->tableStack[program->stackHead++] = table;
    return table;
}

void openScope(Program *program, SymbolTable *scope) {
    assert(program->listCount < MAX_TABLES);
    program->tableStack[program->stackHead++] = scope;
}

SymbolTable *getScope(Program *program) {
    assert(program->stackHead > 0);
    return program->tableStack[program->stackHead - 1];
}

SymbolEntry *findDecl(Program *program, cchar *id) {
    SymbolEntry *result = 0;

    u32 stackHead = program->stackHead;
    while (stackHead && !result) {
        SymbolTable *scope = program->tableStack[--stackHead];
        result = getSymbol(scope, id);
    }

    return result;
}

void closeScope(Program *program) {
    assert(program->stackHead > 0);
    --program->stackHead;
}

void addDeclarations(Program *program, DeclContext *decl) {
    SymbolTable *scope = getScope(program);

    while (decl && !decl->empty()) {
        if (decl->stringDecl()) {
            StringDeclContext *stringDecl = decl->stringDecl();
            assert(stringDecl);

            char *id = saveString(stringDecl->id()->getText().c_str());
            char *value = saveString(stringDecl->str()->getText().c_str());

            if (!addVar(scope, id, 's', value) && !program->firstError) {
                program->firstError = id;
            }
        }
        else {
            VarDeclContext *varDecl = decl->varDecl();
            assert(varDecl);

            char type = tolower(varDecl->varType()->getText()[0]);

            IdListContext *idList = varDecl->idList();
            char *id = saveString(idList->id()->getText().c_str());

            if (!addVar(scope, id, type, 0) && !program->firstError) {
                program->firstError = id;
            }

            IdTailContext *idTail = idList->idTail();
            while (idTail && idTail->id()) {
                id = saveString(idTail->id()->getText().c_str());

                if (!addVar(scope, id, type, 0) && !program->firstError) {
                    program->firstError = id;
                }

                idTail = idTail->idTail();
            }
        }

        decl = decl->decl();
        assert(decl);
    }
}

AstExpr *astFromExprPrefix(Program *, ExprPrefixContext *);
AstExpr *astFromFactor(Program *, FactorContext *);

static inline
AstExpr *_astFromExpr(Program *program, ExprPrefixContext *prefix, FactorContext *factor) {
    assert(prefix);
    assert(factor);

    AstExpr *result = 0;

    if (!prefix->empty()) {
        ExprType type = EXPR_Null;

        if (prefix->addop()->PLUS()) {
            type = EXPR_Addition;
        }
        else {
            assert(prefix->addop()->MINUS());
            type = EXPR_Subtraction;
        }

        result = (AstExpr *)malloc(sizeof *result);
        *result = (AstExpr){
            .type = type,
            .logicalType = 0,
            .tempNumber = getNewTempNumber(program),
            .asBinaryOp = {
                .leftChild = astFromExprPrefix(program, prefix),
                .rightChild = astFromFactor(program, factor),
            },
        };

        result->logicalType = result->asBinaryOp.leftChild->logicalType;
        if (result->asBinaryOp.rightChild->logicalType == 'f') {
            result->logicalType = 'f';
        }
    }
    else {
        result = astFromFactor(program, factor);
    }

    return result;
}

AstExpr *astFromFactorPrefix(Program *, FactorPrefixContext *);
AstExpr *astFromPostfixExpr(Program *, PostfixExprContext *);

static inline
AstExpr *_astFromFactor(Program *program, FactorPrefixContext *prefix, PostfixExprContext *postfix) {
    assert(program);
    assert(prefix);
    assert(postfix);

    AstExpr *result = 0;

    if (!prefix->empty()) {
        ExprType type = EXPR_Null;

        if (prefix->mulop()->STAR()) {
            type = EXPR_Multiplication;
        }
        else {
            assert(prefix->mulop()->SLASH());
            type = EXPR_Division;
        }

        result = (AstExpr *)malloc(sizeof *result);
        *result = (AstExpr){
            .type = type,
            .logicalType = 0,
            .tempNumber = getNewTempNumber(program),
            .asBinaryOp = {
                .leftChild = astFromFactorPrefix(program, prefix),
                .rightChild = astFromPostfixExpr(program, postfix),
            },
        };

        result->logicalType = result->asBinaryOp.leftChild->logicalType;
        if (result->asBinaryOp.rightChild->logicalType == 'f') {
            result->logicalType = 'f';
        }
    }
    else {
        result = astFromPostfixExpr(program, postfix);
    }

    return result;
}

AstExpr *astFromId(Program *program, IdContext *id) {
    assert(program);
    assert(id);

    SymbolEntry *symbol = findDecl(program, id->getText().c_str());
    assert(symbol); /* TODO: don't assert, check! */

    AstExpr *result = (AstExpr *)malloc(sizeof *result);
    *result = (AstExpr){
        .type = EXPR_Symbol,
        .logicalType = symbol->logicalType,
        .tempNumber = getNewTempNumber(program),
        .asTerminal = {
            .nextParam = 0,
            .text = symbol->id,
        }
    };

    return result;
}

AstExpr *astFromIdList(Program *program, IdListContext *list) {
    assert(program);
    assert(list);

    AstExpr *result = 0;
    AstExpr **resultPtr = &result;

    IdContext *id = list->id();

    *resultPtr = astFromId(program, id);
    resultPtr = &(**resultPtr).asTerminal.nextParam;

    IdTailContext *tail = list->idTail();
    while (!tail->empty()) {
        id = tail->id();

        *resultPtr = astFromId(program, id);
        resultPtr = &(**resultPtr).asTerminal.nextParam;

        tail = tail->idTail();
        assert(tail);
    }

    return result;
}

AstExpr *astFromExprPrefix(Program *program, ExprPrefixContext *prefix) {
    assert(program);
    assert(prefix);

    ExprPrefixContext *subPrefix = prefix->exprPrefix();
    FactorContext *factor = prefix->factor();
    return _astFromExpr(program, subPrefix, factor);
}

AstExpr *astFromExpr(Program *, ExprContext *);

AstExpr *astFromPostfixExpr(Program *program, PostfixExprContext *postfix) {
    assert(program);
    assert(postfix);

    AstExpr *result = 0;

    if (postfix->primary()) {
        PrimaryContext *primary = postfix->primary();

        if (primary->expr()) {
            result = astFromExpr(program, primary->expr());
        }
        else {
            result = (AstExpr *)malloc(sizeof *result);
            *result = (AstExpr){
                .type = EXPR_Null, /* to be filled in later */
                .logicalType = 0,  /* to be filled in later */
                .tempNumber = getNewTempNumber(program),
                .asTerminal = {
                    .nextParam = 0, /* not needed here */
                    .text = 0, /* to be filled in later */
                },
            };

            if (primary->id()) {
                IdContext *id = primary->id();

                SymbolEntry *symbol = findDecl(program, id->getText().c_str());
                assert(symbol); /* TODO: don't assert, check! */

                result->type = EXPR_Symbol;
                result->logicalType = symbol->logicalType;
                result->asTerminal.text = symbol->id;
            }
            else if (primary->INTLITERAL()) {
                TerminalNode *intLiteral = primary->INTLITERAL();
                char *text = saveString(intLiteral->getText().c_str());

                result->type = EXPR_IntLiteral;
                result->logicalType = 'i';
                result->asTerminal.text = text;
            }
            else if (primary->FLOATLITERAL()) {
                TerminalNode *floatLiteral = primary->FLOATLITERAL();
                char *text = saveString(floatLiteral->getText().c_str());

                result->type = EXPR_FloatLiteral;
                result->logicalType = 'f';
                result->asTerminal.text = text;
            }
            else { InvalidCodePath; }
        }
    }
    else {
        CallExprContext *call = postfix->callExpr();
        assert(call);

        result = (AstExpr *)malloc(sizeof *result);
        *result = (AstExpr){
            .type = EXPR_Function,
            /* TODO: pull function return type out of scope */
            .logicalType = 0,
            .tempNumber = getNewTempNumber(program),
            .asFuncCall = {
                /* TODO: pull function name out of scope */
                .functionName = saveString(call->id()->getText().c_str()),
                /* TODO: take params */
                .firstParam = 0,
            },
        };
    }

    return result;
}

AstExpr *astFromFactorPrefix(Program *program, FactorPrefixContext *prefix) {
    assert(program);
    assert(prefix);

    FactorPrefixContext *subPrefix = prefix->factorPrefix();
    PostfixExprContext *postfix = prefix->postfixExpr();

    return _astFromFactor(program, subPrefix, postfix);
}

AstExpr *astFromFactor(Program *program, FactorContext *factor) {
    assert(program);
    assert(factor);

    FactorPrefixContext *prefix = factor->factorPrefix();
    PostfixExprContext *postfix = factor->postfixExpr();

    return _astFromFactor(program, prefix, postfix);
}

AstExpr *astFromExpr(Program *program, ExprContext *expr) {
    assert(program);
    assert(expr);

    ExprPrefixContext *prefix = expr->exprPrefix();
    FactorContext *factor = expr->factor();

    return _astFromExpr(program, prefix, factor);
}

AstExpr *astFromCond(Program *program, CondContext *cond) {
    CompopContext *op = cond->compop();
    ExprContext *left = cond->expr(0);
    ExprContext *right = cond->expr(1);

    ExprType type = EXPR_Null;

    if (op->LT()) {
        type = EXPR_LessThan;
    }
    else if (op->GT()) {
        type = EXPR_GreaterThan;
    }
    else if (op->EQUAL()) {
        type = EXPR_Equal;
    }
    else if (op->NOTEQ()) {
        type = EXPR_NotEqual;
    }
    else if (op->LTEQ()) {
        type = EXPR_LessThanOrEqual;
    }
    else if (op->GTEQ()) {
        type = EXPR_GreaterThanOrEqual;
    }
    else { InvalidCodePath; }

    AstExpr *result = (AstExpr *)malloc(sizeof *result);
    *result = (AstExpr){
        .type = type,
        .logicalType = 0,
        .tempNumber = getNewTempNumber(program),
        .asBinaryOp = {
            .leftChild = astFromExpr(program, left),
            .rightChild = astFromExpr(program, right),
        },
    };

    result->logicalType = result->asBinaryOp.leftChild->logicalType;
    if (result->asBinaryOp.rightChild->logicalType == 'f') {
        result->logicalType = 'f';
    }

    return result;
}

AstStatement *addStatements(Program *program, StmtListContext *stmtList) {
    //SymbolTable *scope = getScope(program);
    AstStatement *result = 0;
    AstStatement **resultPtr = &result;

    while (!stmtList->empty()) {
        StmtContext *stmt = stmtList->stmt();

        if (stmt->baseStmt()) {
            BaseStmtContext *base = stmt->baseStmt();
            *resultPtr = makeStatement(STATEMENT_Null);

            if (base->assignStmt()) {
                AssignStmtContext *assignStmt = base->assignStmt();
                AssignExprContext *assignExpr = assignStmt->assignExpr();
                IdContext *id = assignExpr->id();
                ExprContext *expr = assignExpr->expr();

                SymbolEntry *symbol = findDecl(program, id->getText().c_str());
                assert(symbol); /* TODO: don't assert, check! */

                **resultPtr = (AstStatement){
                    .nextStatement = 0,
                    .type = STATEMENT_Assign,
                    .asAssign = {
                        .symbol = symbol->id,
                        .expr = astFromExpr(program, expr),
                    },
                };
            }
            else if (base->readStmt()) {
                ReadStmtContext *readStmt = base->readStmt();

                **resultPtr = (AstStatement){
                    .nextStatement = 0,
                    .type = STATEMENT_Read,
                    .asRead = astFromIdList(program, readStmt->idList()),
                };
            }
            else if (base->writeStmt()) {
                WriteStmtContext *writeStmt = base->writeStmt();

                **resultPtr = (AstStatement){
                    .nextStatement = 0,
                    .type = STATEMENT_Write,
                    .asWrite = astFromIdList(program, writeStmt->idList()),
                };
            }
            else if (base->returnStmt()) {
                ExprContext *expr = base->returnStmt()->expr();

                **resultPtr = (AstStatement){
                    .nextStatement = 0,
                    .type = STATEMENT_Return,
                    .asReturn = astFromExpr(program, expr),
                };
            }
            else { InvalidCodePath; }

            resultPtr = &(**resultPtr).nextStatement;
        }
        else if (stmt->ifStmt()) {
            IfStmtContext *ifStmt = stmt->ifStmt();

            cchar *blockName = getNewBlockName(program);
            SymbolTable *ifScope = openNewScope(program, blockName);
            *resultPtr = makeStatement(STATEMENT_Root);

            addDeclarations(program, ifStmt->decl());

            StmtListContext *stmtList = ifStmt->stmtList();
            AstIfRoot *ifRoot = (AstIfRoot *)malloc(sizeof *ifRoot);
            *ifRoot = (AstIfRoot){
                .header = {
                    .type = ROOT_If,
                    .symbols = ifScope,
                    .firstStatement = addStatements(program, stmtList),
                },
                .comparison = astFromCond(program, ifStmt->cond()),
                .elsePart = 0,
            };

            (**resultPtr).asRoot = (AstRoot *)ifRoot;
            resultPtr = &(**resultPtr).nextStatement;
            closeScope(program);

            ElsePartContext *elseStmt = ifStmt->elsePart();
            if (!elseStmt->empty()) {
                cchar *blockName = getNewBlockName(program);
                SymbolTable *elseScope = openNewScope(program, blockName);

                addDeclarations(program, elseStmt->decl());

                StmtListContext *stmtList = elseStmt->stmtList();
                AstRoot *elseRoot = (AstRoot *)malloc(sizeof *ifRoot);
                *elseRoot = (AstRoot){
                    .type = ROOT_Else,
                    .symbols = elseScope,
                    .firstStatement = addStatements(program, stmtList),
                };

                ifRoot->elsePart = elseRoot;
                closeScope(program);
            }
        }
        else if (stmt->whileStmt()) {
            WhileStmtContext *whileStmt = stmt->whileStmt();

            cchar *blockName = getNewBlockName(program);
            SymbolTable *whileScope = openNewScope(program, blockName);
            *resultPtr = makeStatement(STATEMENT_Root);

            addDeclarations(program, whileStmt->decl());

            StmtListContext *stmtList = whileStmt->stmtList();
            AstWhileRoot *whileRoot = (AstWhileRoot *)malloc(sizeof *whileRoot);
            *whileRoot = (AstWhileRoot){
                .header = {
                    .type = ROOT_While,
                    .symbols = whileScope,
                    .firstStatement = addStatements(program, stmtList),
                },
                .comparison = astFromCond(program, whileStmt->cond()),
            };

            (**resultPtr).asRoot = (AstRoot *)whileRoot;
            resultPtr = &(**resultPtr).nextStatement;
            closeScope(program);
        }
        else { InvalidCodePath; }

        stmtList = stmtList->stmtList();
    }

    return result;
}

void freeExpr(AstExpr *expr) {
    do {
        AstExpr *nextToFree = 0;

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
            freeExpr(expr->asBinaryOp.leftChild);
            freeExpr(expr->asBinaryOp.rightChild);
            free((void *)expr);
            break;

        case EXPR_IntLiteral:
        case EXPR_FloatLiteral:
        case EXPR_StringLiteral:
        case EXPR_Symbol:
            nextToFree = expr->asTerminal.nextParam;
            free((void *)expr);
            break;

        case EXPR_Function:
            /* TODO: free params */
            free((void *)expr);
            break;

        case EXPR_Null: InvalidCodePath;
        }

        expr = nextToFree;
    }
    while (expr);
}

void freeRoot(AstRoot *root) {
    assert(root);

    AstStatement *statement = root->firstStatement;
    while (statement) {
        AstStatement *nextStatement = statement->nextStatement;

        switch (statement->type) {
        case STATEMENT_Root:
            freeRoot(statement->asRoot);
            break;

        case STATEMENT_Assign:
            freeExpr(statement->asAssign.expr);
            break;

        case STATEMENT_Return:
            freeExpr(statement->asReturn);
            break;

        case STATEMENT_Read:
            freeExpr(statement->asRead);
            break;

        case STATEMENT_Write:
            freeExpr(statement->asWrite);
            break;

        default: break; /* TODO: the rest */
        }

        free((void *) statement);
        statement = nextStatement;
    }

    if (root->type == ROOT_If) {
        AstIfRoot *ifRoot = (AstIfRoot *)root;
        AstRoot *elseRoot = ifRoot->elsePart;

        if (elseRoot) {
            freeRoot(elseRoot);
        }

        freeExpr(ifRoot->comparison);
    }
    else if (root->type == ROOT_While) {
        AstWhileRoot *whileRoot = (AstWhileRoot *)root;
        freeExpr(whileRoot->comparison);
    }

    free((void *)root);
}

AstRoot *makeFuncRoot(Program *program, FuncDeclContext *ctx, cchar *id) {
    assert(program);
    assert(ctx);
    assert(id);

    SymbolTable *scope = openNewScope(program, id);

    AstRoot *root = (AstRoot *)malloc(sizeof *root);
    *root = (AstRoot){
        .type = ROOT_Function,
        .symbols = scope,
        .firstStatement = 0,
    };

    /*
     * Parameters
     */

    ParamDeclListContext *paramList = ctx->paramDeclList();
    if (!paramList->empty()) {
        ParamDeclContext *param = paramList->paramDecl();

        char *id = saveString(param->id()->getText().c_str());
        char type = tolower(param->varType()->getText()[0]);

        if (!addParam(scope, id, type) && !program->firstError) {
            program->firstError = id;
        }

        ParamDeclTailContext *paramTail = paramList->paramDeclTail();
        while (!paramTail->empty()) {
            param = paramTail->paramDecl();

            id = saveString(param->id()->getText().c_str());
            type = tolower(param->varType()->getText()[0]);

            if (!addParam(scope, id, type) && !program->firstError) {
                program->firstError = id;
            }

            paramTail = paramTail->paramDeclTail();
            assert(paramTail);
        }
    }

    /*
     * Declarations
     */

    FuncBodyContext *funcBody = ctx->funcBody();
    addDeclarations(program, funcBody->decl());

    /*
     * Walk the Statements
     */

    root->firstStatement = addStatements(program, funcBody->stmtList());

    closeScope(program);

    return root;
}

void freeProgram(Program *program) {
    assert(program);

    for (u32 listIndex = 0; listIndex < program->listCount; ++listIndex) {
        SymbolTable *table = program->tableList + listIndex;

        /* get rid of any roots */
        for (u32 i = 0; i < table->count; ++i) {
            u32 index = table->order[i];
            SymbolEntry entry = table->data[index];

            if (entry.root) {
                freeRoot(entry.root);
            }
        }

        deinitSymbolTable(table);
    }

    free((void *)program);
}

Program *makeProgram(FileContext *ctx) {
    Program *program = (Program *)malloc(sizeof *program);
    *program = (Program){
        .root = (AstRoot){
            .type = ROOT_Global,
            .symbols = 0,
            .firstStatement = 0,
        },
        .firstError = 0,
        .blockCount = 0,
        .tempCount = 0,
        .listCount = 0,
        .stackHead = 0,
        .tableList = {},
        .tableStack = {},
    };

    PgmBodyContext *pgmBody = ctx->program()->pgmBody();
    SymbolTable *scope = openNewScope(program, "GLOBAL");
    program->root.symbols = scope;

    /* TODO: check for errors on our way down */

    /*
     * Declarations
     */

    addDeclarations(program, pgmBody->decl());

    /*
     * Functions
     */

    FuncDeclarationsContext *funcs = pgmBody->funcDeclarations();
    while (!funcs->empty()) {
        FuncDeclContext *funcDecl = funcs->funcDecl();
        assert(funcDecl);

        char returnType = tolower(funcDecl->anyType()->getText()[0]);
        cchar *id = saveString(funcDecl->id()->getText().c_str());
        char paramTypes[32] = {};
        /* NOTE: 31 parameters should be enough for anyone */

        /*
         * build up the parameter type string
         */

        char *cur = paramTypes;
        ParamDeclListContext *paramList = funcDecl->paramDeclList();
        if (!paramList->empty()) {
            ParamDeclContext *param = paramList->paramDecl();

            *cur++ = tolower(param->varType()->getText()[0]);
            assert(cur < paramTypes + sizeof paramTypes - 1);

            ParamDeclTailContext *paramTail = paramList->paramDeclTail();
            while (!paramTail->empty()) {
                param = paramTail->paramDecl();

                *cur++ = tolower(param->varType()->getText()[0]);
                assert(cur < paramTypes + sizeof paramTypes - 1);

                paramTail = paramTail->paramDeclTail();
                assert(paramTail);
            }
        }

        AstRoot *funcRoot = makeFuncRoot(program, funcDecl, id);
        addFunc(scope, id, returnType, saveString(paramTypes), funcRoot);

        funcs = funcs->funcDeclarations();
        assert(funcs);
    }

    closeScope(program);

    return program;
}
