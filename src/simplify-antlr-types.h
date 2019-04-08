#ifndef simplify_antlr_types_h
#define simplify_antlr_types_h

#include <TINYParser.h>

typedef TINYParser::FileContext             FileContext;
typedef TINYParser::ProgramContext          ProgramContext;
typedef TINYParser::IdContext               IdContext;
typedef TINYParser::PgmBodyContext          PgmBodyContext;
typedef TINYParser::DeclContext             DeclContext;
typedef TINYParser::StringDeclContext       StringDeclContext;
typedef TINYParser::StrContext              StrContext;
typedef TINYParser::VarDeclContext          VarDeclContext;
typedef TINYParser::VarTypeContext          VarTypeContext;
typedef TINYParser::AnyTypeContext          AnyTypeContext;
typedef TINYParser::IdListContext           IdListContext;
typedef TINYParser::IdTailContext           IdTailContext;
typedef TINYParser::ParamDeclListContext    ParamDeclListContext;
typedef TINYParser::ParamDeclContext        ParamDeclContext;
typedef TINYParser::ParamDeclTailContext    ParamDeclTailContext;
typedef TINYParser::FuncDeclarationsContext FuncDeclarationsContext;
typedef TINYParser::FuncDeclContext         FuncDeclContext;
typedef TINYParser::FuncBodyContext         FuncBodyContext;
typedef TINYParser::StmtListContext         StmtListContext;
typedef TINYParser::StmtContext             StmtContext;
typedef TINYParser::BaseStmtContext         BaseStmtContext;
typedef TINYParser::AssignStmtContext       AssignStmtContext;
typedef TINYParser::AssignExprContext       AssignExprContext;
typedef TINYParser::ReadStmtContext         ReadStmtContext;
typedef TINYParser::WriteStmtContext        WriteStmtContext;
typedef TINYParser::ReturnStmtContext       ReturnStmtContext;
typedef TINYParser::ExprContext             ExprContext;
typedef TINYParser::ExprPrefixContext       ExprPrefixContext;
typedef TINYParser::FactorContext           FactorContext;
typedef TINYParser::FactorPrefixContext     FactorPrefixContext;
typedef TINYParser::PostfixExprContext      PostfixExprContext;
typedef TINYParser::CallExprContext         CallExprContext;
typedef TINYParser::ExprListContext         ExprListContext;
typedef TINYParser::ExprListTailContext     ExprListTailContext;
typedef TINYParser::PrimaryContext          PrimaryContext;
typedef TINYParser::AddopContext            AddopContext;
typedef TINYParser::MulopContext            MulopContext;
typedef TINYParser::IfStmtContext           IfStmtContext;
typedef TINYParser::ElsePartContext         ElsePartContext;
typedef TINYParser::CondContext             CondContext;
typedef TINYParser::CompopContext           CompopContext;
typedef TINYParser::WhileStmtContext        WhileStmtContext;
typedef TINYParser::EmptyContext            EmptyContext;


#endif
