#include "front/syntax.h"

#include <cassert>
#include <iostream>

using frontend::Parser;

// #define DEBUG_PARSER
#define TODO assert(0 && "todo")
#define CUR_TOKEN_IS(tk_type) (token_stream[index].type == TokenType::tk_type)
#define PARSE_TOKEN(tk_type) root->children.push_back(parseTerm(root, TokenType::tk_type))
#define PARSE(name, type)                                                                          \
    auto name = new type(root);                                                                    \
    assert(parse##type(name));                                                                     \
    root->children.push_back(name);

Parser::Parser(const std::vector<frontend::Token>& tokens) : index(0), token_stream(tokens) {}

Parser::~Parser() {}

frontend::CompUnit* Parser::get_abstract_syntax_tree()
{
    CompUnit* p = new CompUnit();
    parseCompUnit(p);
    return (CompUnit*)p->children[0];
}

void Parser::log(AstNode* node)
{
#ifdef DEBUG_PARSER
    std::cout << "in parse" << toString(node->type)
              << ", cur_token_type::" << toString(token_stream[index].type)
              << ", token_val::" << token_stream[index].value << '\n';
#endif
}
/*



*/
#define MATCHFAIL                                                                                  \
    {                                                                                              \
        this->index = Traceid;                                                                     \
        delete p;                                                                                  \
        return false;                                                                              \
    }
#define MATCHSUCCESS                                                                               \
    {                                                                                              \
        root->children.push_back(p);                                                               \
        p->parent = root;                                                                          \
        return true;                                                                               \
    }

#define INIT(type)                                                                                 \
    uint32_t Traceid = this->index;                                                                \
    type* p          = new type(root)

#define TRACEBACK(type)                                                                            \
    {                                                                                              \
        delete p;                                                                                  \
        p           = new type();                                                                  \
        this->index = Traceid;                                                                     \
    }
bool Parser::parseTerm(AstNode* root, TokenType expect)
{
    if (this->index == this->token_stream.size())
        return false;
    Token token = this->token_stream[this->index];
    if (expect != token.type)
        return false;
    this->index++;
    Term* p = new Term(token, root);
    MATCHSUCCESS;
}
bool Parser::parseCompUnit(AstNode* root)  // new
{
    INIT(CompUnit);
    if (parseDecl(p)) {
        parseCompUnit(p);

        MATCHSUCCESS;
    }
    TRACEBACK(CompUnit);
    if (parseFuncDef(p)) {
        parseCompUnit(p);
        MATCHSUCCESS;
    }
    MATCHFAIL;
}
bool Parser::parseDecl(AstNode* root)  // new
{
    INIT(Decl);
    if (parseConstDecl(p))
        MATCHSUCCESS;
    TRACEBACK(Decl);
    if (parseVarDecl(p))
        MATCHSUCCESS;
    MATCHFAIL;
}
bool Parser::parseFuncDef(AstNode* root)  // new
{
    INIT(FuncDef);
    if (parseFuncType(p) && parseTerm(p, TokenType::IDENFR) && parseTerm(p, TokenType::LPARENT)) {
        parseFuncFParams(p);
        if (parseTerm(p, TokenType::RPARENT) && parseBlock(p))
            MATCHSUCCESS;
    }
    MATCHFAIL;
}
bool Parser::parseConstDecl(AstNode* root)  // new
{
    INIT(ConstDecl);
    if (parseTerm(p, TokenType::CONSTTK) && parseBType(p) && parseConstDef(p)) {
        while (parseTerm(p, TokenType::COMMA))
            assert(parseConstDef(p));
        if (parseTerm(p, TokenType::SEMICN))
            MATCHSUCCESS;
    }
    MATCHFAIL;
}
bool Parser::parseBType(AstNode* root)  // new
{
    INIT(BType);
    if (parseTerm(p, TokenType::INTTK))
        MATCHSUCCESS;
    TRACEBACK(BType);
    if (parseTerm(p, TokenType::FLOATTK))
        MATCHSUCCESS;
    MATCHFAIL;
}
bool Parser::parseConstDef(AstNode* root)  // new
{
    INIT(ConstDef);
    if (parseTerm(p, TokenType::IDENFR)) {
        while (parseTerm(p, TokenType::LBRACK))
            assert(parseConstExp(p) && parseTerm(p, TokenType::RBRACK));
        if (parseTerm(p, TokenType::ASSIGN) && parseConstInitVal(p))
            MATCHSUCCESS;
    }
    MATCHFAIL;
}
bool Parser::parseConstInitVal(AstNode* root)  // new
{
    INIT(ConstInitVal);
    if (parseTerm(p, TokenType::LBRACE)) {
        if (parseConstInitVal(p))
            while (parseTerm(p, TokenType::COMMA))
                assert(parseConstInitVal(p));
        if (parseTerm(p, TokenType::RBRACE))
            MATCHSUCCESS;
    }
    TRACEBACK(ConstInitVal);
    if (parseConstExp(p))
        MATCHSUCCESS;
    MATCHFAIL;
}
bool Parser::parseVarDecl(AstNode* root)  // new
{
    INIT(VarDecl);
    if (parseBType(p) && parseVarDef(p)) {
        while (parseTerm(p, TokenType::COMMA))
            assert(parseVarDef(p));
        if (parseTerm(p, TokenType::SEMICN))
            MATCHSUCCESS;
    }
    MATCHFAIL;
}
bool Parser::parseVarDef(AstNode* root)  // new
{
    uint32_t Traceid = this->index;
    VarDef* p        = new VarDef();
    if (!parseTerm(p, TokenType::IDENFR))
        MATCHFAIL;
    while (parseTerm(p, TokenType::LBRACK))
        if (!parseConstExp(p) || !parseTerm(p, TokenType::RBRACK))
            MATCHFAIL;
    if (parseTerm(p, TokenType::ASSIGN))
        if (!parseInitVal(p))
            MATCHFAIL;
    MATCHSUCCESS;
}
bool Parser::parseInitVal(AstNode* root)  // new
{
    uint32_t Traceid = this->index;
    InitVal* p       = new InitVal();
    if (parseExp(p))
        MATCHSUCCESS;
    if (!parseTerm(p, TokenType::LBRACE))
        MATCHFAIL;
    if (parseInitVal(p))
        while (parseTerm(p, TokenType::COMMA))
            if (!parseInitVal(p))
                MATCHFAIL;
    if (!parseTerm(p, TokenType::RBRACE))
        MATCHFAIL;
    MATCHSUCCESS;
}
bool Parser::parseFuncType(AstNode* root)  // new
{
    INIT(FuncType);
    if (parseTerm(p, TokenType::INTTK))
        MATCHSUCCESS;
    TRACEBACK(FuncType);
    if (parseTerm(p, TokenType::FLOATTK))
        MATCHSUCCESS;
    TRACEBACK(FuncType);
    if (parseTerm(p, TokenType::VOIDTK))
        MATCHSUCCESS;
    MATCHFAIL;
}
bool Parser::parseFuncFParam(AstNode* root)  // new
{
    INIT(FuncFParam);
    if (parseBType(p) && parseTerm(p, TokenType::IDENFR)) {
        if (parseTerm(p, TokenType::LBRACK)) {
            assert(parseTerm(p, TokenType::RBRACK));
            while (parseTerm(p, TokenType::LBRACK))
                assert(parseExp(p) && parseTerm(p, TokenType::RBRACK));
        }
        MATCHSUCCESS
    }
    MATCHFAIL;
}
bool Parser::parseFuncFParams(AstNode* root)  // new
{
    INIT(FuncFParams);
    if (parseFuncFParam(p)) {
        while (parseTerm(p, TokenType::COMMA))
            assert(parseFuncFParam(p));
        MATCHSUCCESS;
    }
    MATCHFAIL;
}
bool Parser::parseBlock(AstNode* root)
{
    INIT(Block);
    if (parseTerm(p, TokenType::LBRACE)) {
        while (parseBlockItem(p))
            ;
        assert(parseTerm(p, TokenType::RBRACE));
        MATCHSUCCESS;
    }
    MATCHFAIL;
}
bool Parser::parseBlockItem(AstNode* root)  // new
{
    INIT(BlockItem);
    if (parseDecl(p))
        MATCHSUCCESS;
    if (parseStmt(p))
        MATCHSUCCESS;
    MATCHFAIL;
}
bool Parser::parseStmt(AstNode* root)  // new
{
    INIT(Stmt);
    if (parseTerm(p, TokenType::IFTK)) {
        assert(parseTerm(p, TokenType::LPARENT) && parseCond(p) &&
               parseTerm(p, TokenType::RPARENT) && parseStmt(p));
        if (parseTerm(p, TokenType::ELSETK))
            assert(parseStmt(p));
        MATCHSUCCESS;
    }
    TRACEBACK(Stmt);
    if (parseTerm(p, TokenType::WHILETK)) {
        assert(parseTerm(p, TokenType::LPARENT) && parseCond(p) &&
               parseTerm(p, TokenType::RPARENT) && parseStmt(p));
        MATCHSUCCESS;
    }
    TRACEBACK(Stmt);
    if (parseTerm(p, TokenType::BREAKTK)) {
        assert(parseTerm(p, TokenType::SEMICN));
        MATCHSUCCESS;
    }
    TRACEBACK(Stmt);
    if (parseTerm(p, TokenType::CONTINUETK)) {
        assert(parseTerm(p, TokenType::SEMICN));
        MATCHSUCCESS;
    }
    TRACEBACK(Stmt);
    if (parseTerm(p, TokenType::RETURNTK)) {
        parseExp(p);
        assert(parseTerm(p, TokenType::SEMICN));
        MATCHSUCCESS;
    }
    TRACEBACK(Stmt);
    if (parseTerm(p, TokenType::SEMICN))
        MATCHSUCCESS;
    TRACEBACK(Stmt);
    if (parseLVal(p)) {
        if (parseTerm(p, TokenType::ASSIGN) && parseExp(p) && parseTerm(p, TokenType::SEMICN))
            MATCHSUCCESS;
    }
    TRACEBACK(Stmt);
    if (parseBlock(p))
        MATCHSUCCESS;
    TRACEBACK(Stmt);

    if (parseExp(p))
        if (parseTerm(p, TokenType::SEMICN))
            MATCHSUCCESS;
    MATCHFAIL;
}
bool Parser::parseExp(AstNode* root)
{
    uint32_t Traceid = this->index;
    Exp* p           = new Exp();
    if (!parseAddExp(p))
        MATCHFAIL;
    MATCHSUCCESS;
}
bool Parser::parseCond(AstNode* root)
{
    uint32_t Traceid = this->index;
    Cond* p          = new Cond();
    if (!parseLOrExp(p))
        MATCHFAIL;
    MATCHSUCCESS;
}
bool Parser::parseLVal(AstNode* root)
{
    uint32_t Traceid = this->index;
    LVal* p          = new LVal();
    if (!parseTerm(p, TokenType::IDENFR))
        MATCHFAIL;
    while (parseTerm(p, TokenType::LBRACK))
        if (!parseExp(p) || !parseTerm(p, TokenType::RBRACK))
            MATCHFAIL;
    MATCHSUCCESS;
}
bool Parser::parseNumber(AstNode* root)
{
    uint32_t Traceid = this->index;
    Number* p        = new Number();
    if (parseTerm(p, TokenType::INTLTR))
        MATCHSUCCESS;
    if (!parseTerm(p, TokenType::FLOATLTR))
        MATCHFAIL;
    MATCHSUCCESS;
}
bool Parser::parsePrimaryExp(AstNode* root)
{
    uint32_t Traceid = this->index;
    PrimaryExp* p    = new PrimaryExp();
    if (parseTerm(p, TokenType::LPARENT)) {
        if (!parseExp(p) || !parseTerm(p, TokenType::RPARENT))
            MATCHFAIL;
        MATCHSUCCESS;
    }
    if (parseLVal(p))
        MATCHSUCCESS;
    if (parseNumber(p))
        MATCHSUCCESS;
    MATCHFAIL;
}
bool Parser::parseUnaryExp(AstNode* root)
{
    uint32_t Traceid = this->index;
    UnaryExp* p      = new UnaryExp();
    if (parseTerm(p, TokenType::IDENFR)) {
        if (parseTerm(p, TokenType::LPARENT) && (parseFuncRParams(p) || 1) &&
            parseTerm(p, TokenType::RPARENT))
            MATCHSUCCESS;
        delete p;
        p           = new UnaryExp();
        this->index = Traceid;
    }
    if (parsePrimaryExp(p))
        MATCHSUCCESS;
    if (!parseUnaryOp(p) || !parseUnaryExp(p))
        MATCHFAIL;
    MATCHSUCCESS;
}
bool Parser::parseUnaryOp(AstNode* root)
{
    uint32_t Traceid = this->index;
    UnaryOp* p       = new UnaryOp();
    if (parseTerm(p, TokenType::PLUS))
        MATCHSUCCESS;
    if (parseTerm(p, TokenType::MINU))
        MATCHSUCCESS;
    if (parseTerm(p, TokenType::NOT))
        MATCHSUCCESS;
    MATCHFAIL;
}
bool Parser::parseFuncRParams(AstNode* root)
{
    uint32_t Traceid = this->index;
    FuncRParams* p   = new FuncRParams();
    if (!parseExp(p))
        MATCHFAIL;
    while (parseTerm(p, TokenType::COMMA)) {
        if (!parseExp(p))
            MATCHFAIL;
    }
    MATCHSUCCESS;
}
bool Parser::parseMulExp(AstNode* root)
{
    uint32_t Traceid = this->index;
    MulExp* p        = new MulExp();
    if (!parseUnaryExp(p))
        MATCHFAIL;
    while (parseTerm(p, TokenType::MULT) || parseTerm(p, TokenType::DIV) ||
           parseTerm(p, TokenType::MOD)) {
        if (!parseUnaryExp(p))
            MATCHFAIL;
    }
    MATCHSUCCESS;
}
bool Parser::parseAddExp(AstNode* root)
{
    uint32_t Traceid = this->index;
    AddExp* p        = new AddExp();
    if (!parseMulExp(p))
        MATCHFAIL;
    while (parseTerm(p, TokenType::PLUS) || parseTerm(p, TokenType::MINU)) {
        if (!parseMulExp(p))
            MATCHFAIL;
    }
    MATCHSUCCESS;
}
bool Parser::parseRelExp(AstNode* root)
{
    uint32_t Traceid = this->index;
    RelExp* p        = new RelExp();
    if (!parseAddExp(p))
        MATCHFAIL;
    while (parseTerm(p, TokenType::LEQ) || parseTerm(p, TokenType::GEQ) ||
           parseTerm(p, TokenType::GTR) || parseTerm(p, TokenType::LSS)) {
        if (!parseAddExp(p))
            MATCHFAIL;
    }
    MATCHSUCCESS;
}
bool Parser::parseEqExp(AstNode* root)
{
    uint32_t Traceid = this->index;
    EqExp* p         = new EqExp();
    if (!parseRelExp(p))
        MATCHFAIL;
    while (parseTerm(p, TokenType::EQL) || parseTerm(p, TokenType::NEQ)) {
        if (!parseRelExp(p))
            MATCHFAIL;
    }
    MATCHSUCCESS;
}
bool Parser::parseLAndExp(AstNode* root)
{
    uint32_t Traceid = this->index;
    LAndExp* p       = new LAndExp();
    if (!parseEqExp(p))
        MATCHFAIL;
    if (parseTerm(p, TokenType::AND)) {
        if (!parseLAndExp(p))
            MATCHFAIL;
    }
    MATCHSUCCESS;
}
bool Parser::parseLOrExp(AstNode* root)
{
    uint32_t Traceid = this->index;
    LOrExp* p        = new LOrExp();
    if (!parseLAndExp(p))
        MATCHFAIL;
    if (parseTerm(p, TokenType::OR)) {
        if (!parseLOrExp(p))
            MATCHFAIL;
    }
    MATCHSUCCESS;
}
bool Parser::parseConstExp(AstNode* root)
{
    uint32_t Traceid = this->index;
    ConstExp* p      = new ConstExp();
    if (!parseAddExp(p))
        MATCHFAIL;
    MATCHSUCCESS;
}