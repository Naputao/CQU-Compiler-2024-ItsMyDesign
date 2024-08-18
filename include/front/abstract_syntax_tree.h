/**
 * @file AST.h
 * @author Yuntao Dai (d1581209858@live.com)
 * @brief
 * abstract syntax tree
 * there is a basic class AstNode,
 * and for every non-terminal lexical unit, we create a sub-class for it,
 * sub-class should implement the IR generating function for itself
 * @version 0.1
 * @date 2022-12-19
 *
 * @copyright Copyright (c) 2022
 *
 */

#ifndef AST_H
#define AST_H

#include "front/token.h"
#include "ir/ir.h"
#include "json/json.h"
using ir::Operand;
using ir::Type;
#include <string>
#include <vector>
using std::string;
using std::vector;

namespace frontend {

// enumerate for node type
enum class NodeType {
    TERMINAL,  // terminal lexical unit
    COMPUINT,
    DECL,
    FUNCDEF,
    CONSTDECL,
    BTYPE,
    CONSTDEF,
    CONSTINITVAL,
    VARDECL,
    VARDEF,
    INITVAL,
    FUNCTYPE,
    FUNCFPARAM,
    FUNCFPARAMS,
    BLOCK,
    BLOCKITEM,
    STMT,
    EXP,
    COND,
    LVAL,
    NUMBER,
    PRIMARYEXP,
    UNARYEXP,
    UNARYOP,
    FUNCRPARAMS,
    MULEXP,
    ADDEXP,
    RELEXP,
    EQEXP,
    LANDEXP,
    LOREXP,
    CONSTEXP,
};
std::string toString(NodeType);

struct Varient
{
    Type t;
    Operand v_int;
    Operand v_float;
};

// tree node basic class
struct AstNode
{
    NodeType type;              // the node type
    AstNode* parent;            // the parent node
    vector<AstNode*> children;  // children of node

    /**
     * @brief constructor
     */
    AstNode(NodeType t, AstNode* p = nullptr);

    /**
     * @brief destructor
     */
    virtual ~AstNode();

    /**
     * @brief Get the json output object
     * @param root: a Json::Value buffer, should be initialized before calling
     * this function
     */
    void get_json_output(Json::Value& root) const;

    // rejcet copy and assignment
    AstNode(const AstNode&)            = delete;
    AstNode& operator=(const AstNode&) = delete;
};

struct Term : AstNode
{
    Token token;

    /**
     * @brief constructor
     */
    Term(Token t, AstNode* p = nullptr);
};

struct CompUnit : AstNode
{
    /**
     * @brief constructor
     */
    CompUnit(AstNode* p = nullptr);
};

struct Decl : AstNode
{
    /**
     * @brief constructor
     */
    Decl(AstNode* p = nullptr);
};

struct FuncDef : AstNode
{
    string n;
    Type t;

    /**
     * @brief constructor
     */
    FuncDef(AstNode* p = nullptr);
};

struct ConstDecl : AstNode
{
    Type t;

    /**
     * @brief constructor
     */
    ConstDecl(AstNode* p = nullptr);
};

struct BType : AstNode
{
    Type t;

    /**
     * @brief constructor
     */
    BType(AstNode* p = nullptr);
};

struct ConstDef : AstNode
{
    string arr_name;
    /**
     * @brief constructor
     */
    ConstDef(AstNode* p = nullptr);
};

struct ConstInitVal : AstNode
{
    bool is_computable;
    Type t;
    vector<Operand> vecV;
    vector<Varient> vecVar;
    /**
     * @brief constructor
     */
    ConstInitVal(AstNode* p = nullptr);
};

struct VarDecl : AstNode
{
    Type t;
    /**
     * @brief constructor
     */
    VarDecl(AstNode* p = nullptr);
};

struct VarDef : AstNode
{
    string arr_name;
    /**
     * @brief constructor
     */
    VarDef(AstNode* p = nullptr);
};

struct InitVal : AstNode
{
    bool is_computable;
    Type t;
    vector<Operand> vecV;
    vector<Varient> vecVar;
    /**
     * @brief constructor
     */
    InitVal(AstNode* p = nullptr);
};

struct FuncType : AstNode
{
    Type t;
    string n;
    /**
     * @brief constructor
     */
    FuncType(AstNode* p = nullptr);
};

struct FuncFParam : AstNode
{
    Operand Parameter;
    /**
     * @brief constructor
     */
    FuncFParam(AstNode* p = nullptr);
};

struct FuncFParams : AstNode
{
    std::vector<Operand> ParameterList;
    /**
     * @brief constructor
     */
    FuncFParams(AstNode* p = nullptr);
};

struct Block : AstNode
{
    /**
     * @brief constructor
     */
    Block(AstNode* p = nullptr);
};

struct BlockItem : AstNode
{
    /**
     * @brief constructor
     */
    BlockItem(AstNode* p = nullptr);
};

struct Stmt : AstNode
{
    /**
     * @brief constructor
     */
    Stmt(AstNode* p = nullptr);
};

struct Exp : AstNode
{
    bool is_computable;
    Type t;
    Operand v;
    Varient var;
    /**
     * @brief constructor
     */
    Exp(AstNode* p = nullptr);
};

struct Cond : AstNode
{
    bool is_computable;
    Type t;
    Operand v;
    Varient var;
    /**
     * @brief constructor
     */
    Cond(AstNode* p = nullptr);
};

struct LVal : AstNode
{
    bool is_computable;
    Type t;
    Operand v;
    Operand i;
    Varient var;
    /**
     * @brief constructor
     */
    LVal(AstNode* p = nullptr);
};

struct Number : AstNode
{
    bool is_computable;
    Type t;
    Operand v;
    Varient var;
    /**
     * @brief constructor
     */
    Number(AstNode* p = nullptr);
};

struct PrimaryExp : AstNode
{
    bool is_computable;
    Type t;
    Operand v;
    Varient var;
    /**
     * @brief constructor
     */
    PrimaryExp(AstNode* p = nullptr);
};

struct UnaryExp : AstNode
{
    bool is_computable;
    Type t;
    Operand v;
    Varient var;
    /**
     * @brief constructor
     */
    UnaryExp(AstNode* p = nullptr);
};

struct UnaryOp : AstNode
{
    TokenType tk_t;
    /**
     * @brief constructor
     */
    UnaryOp(AstNode* p = nullptr);
};

struct FuncRParams : AstNode
{
    std::vector<Operand> paraVec;
    /**
     * @brief constructor
     */
    FuncRParams(AstNode* p = nullptr);
};

struct MulExp : AstNode
{
    bool is_computable;
    Type t;
    Operand v;
    Varient var;
    /**
     * @brief constructor
     */
    MulExp(AstNode* p = nullptr);
};

struct AddExp : AstNode
{
    bool is_computable;
    Type t;
    Operand v;
    Varient var;
    /**
     * @brief constructor
     */
    AddExp(AstNode* p = nullptr);
};

struct RelExp : AstNode
{
    bool is_computable;
    Type t;
    Operand v;
    Varient var;
    /**
     * @brief constructor
     */
    RelExp(AstNode* p = nullptr);
};

struct EqExp : AstNode
{
    bool is_computable;
    Type t;
    Operand v;
    Varient var;
    /**
     * @brief constructor
     */
    EqExp(AstNode* p = nullptr);
};

struct LAndExp : AstNode
{
    bool is_computable;
    Type t;
    Operand v;
    Varient var;
    /**
     * @brief constructor
     */
    LAndExp(AstNode* p = nullptr);
};

struct LOrExp : AstNode
{
    bool is_computable;
    Type t;
    Operand v;
    Varient var;
    /**
     * @brief constructor
     */
    LOrExp(AstNode* p = nullptr);
};

struct ConstExp : AstNode
{
    bool is_computable;
    Type t;
    Operand v;
    Varient var;
    /**
     * @brief constructor
     */
    ConstExp(AstNode* p = nullptr);
};

}  // namespace frontend

#endif