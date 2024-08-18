/**
 * @file semantic.h
 * @author Yuntao Dai (d1581209858@live.com)
 * @brief
 * @version 0.1
 * @date 2023-01-06
 *
 * a Analyzer should
 * @copyright Copyright (c) 2023
 *
 */

#ifndef SEMANTIC_H
#define SEMANTIC_H

#include "front/abstract_syntax_tree.h"
#include "ir/ir.h"

#include <map>
#include <string>
#include <vector>
using std::map;
using std::string;
using std::vector;

namespace frontend {

// definition of symbol table entry
struct STE
{
    ir::Operand operand;
    vector<int> dimension;
};

using map_str_ste = map<string, STE>;
// definition of scope infomation
struct ScopeInfo
{
    int cnt;
    string name;
    map_str_ste table;
};

// surpport lib functions
map<std::string, ir::Function*>* get_lib_funcs();

// definition of symbol table
struct SymbolTable
{
    vector<ScopeInfo> scope_stack;
    map<std::string, ir::Function*> functions;

    /**
     * @brief enter a new scope, record the infomation in scope stacks
     * @param node: a Block node, entering a new Block means a new name scope
     */
    void add_scope(Block*);

    /**
     * @brief exit a scope, pop out infomations
     */
    void exit_scope();

    /**
     * @brief Get the scoped name, to deal the same name in different scopes, we
     * change origin id to a new one with scope infomation, for example, we have
     * these code:
     * "
     * int a;
     * {
     *      int a; ....
     * }
     * "
     * in this case, we have two variable both name 'a', after change they will
     * be 'a' and 'a_block'
     * @param id: origin id
     * @return string: new name with scope infomations
     */
    string get_scoped_name(string id) const;

    /**
     * @brief get the right operand with the input name
     * @param id identifier name
     * @return Operand
     */
    ir::Operand get_operand(string id) const;

    /**
     * @brief get the right ste with the input name
     * @param id identifier name
     * @return STE
     */
    STE get_ste(string id) const;
};

// singleton class
struct Analyzer
{
    int tmp_cnt;
    vector<ir::Instruction*> g_init_inst;
    SymbolTable symbol_table;

    /**
     * @brief constructor
     */
    Analyzer();

    // analysis functions
    ir::Program get_ir_program(CompUnit*);

    // reject copy & assignment
    Analyzer(const Analyzer&)            = delete;
    Analyzer& operator=(const Analyzer&) = delete;

    // analysis functions
    void analysisCompUnit(CompUnit* root, ir::Program& program);
    void analysisDecl(Decl* root, vector<ir::Instruction*>& instrs);
    void analysisFuncDef(FuncDef* root, ir::Function& func);
    void analysisConstDecl(ConstDecl* root, vector<ir::Instruction*>& instrs);
    void analysisConstDef(ConstDef* root, vector<ir::Instruction*>& instrs);
    void analysisConstExp(ConstExp* root, vector<ir::Instruction*>& instrs);
    void analysisConstInitVal(ConstInitVal* root, vector<ir::Instruction*>& instrs);
    void analysisVarDecl(VarDecl* root, vector<ir::Instruction*>& instrs);
    void analysisVarDef(VarDef* root, vector<ir::Instruction*>& instrs);
    void analysisInitVal(InitVal* root, vector<ir::Instruction*>& instrs);
    void analysisFuncFParam(FuncFParam* root, ir::Function& func);
    void analysisFuncFParams(FuncFParams* root, ir::Function& func);
    void analysisBlock(Block* root, vector<ir::Instruction*>& instrs);
    void analysisBlockItem(BlockItem* root, vector<ir::Instruction*>& instrs);
    void analysisStmt(Stmt* root, vector<ir::Instruction*>& instrs);
    void analysisExp(Exp* root, vector<ir::Instruction*>& instrs);
    void analysisCond(Cond* root, vector<ir::Instruction*>& instrs);
    void analysisLVal(LVal* root, vector<ir::Instruction*>& instrs);
    void analysisPrimaryExp(PrimaryExp* root, vector<ir::Instruction*>& instrs);
    void analysisUnaryExp(UnaryExp* root, vector<ir::Instruction*>& instrs);
    void analysisFuncRParams(FuncRParams* root, vector<ir::Instruction*>& instrs);
    void analysisMulExp(MulExp* root, vector<ir::Instruction*>& instrs);
    void analysisAddExp(AddExp* root, vector<ir::Instruction*>& instrs);
    void analysisRelExp(RelExp* root, vector<ir::Instruction*>& instrs);
    void analysisEqExp(EqExp* root, vector<ir::Instruction*>& instrs);
    void analysisLAndExp(LAndExp* root, vector<ir::Instruction*>& instrs);
    void analysisLOrExp(LOrExp* root, vector<ir::Instruction*>& instrs);
    void analysisBType(BType* root);
    void analysisFuncType(FuncType* root);
    void analysisNumber(Number* root);
    void analysisUnaryOp(UnaryOp* root);
};

}  // namespace frontend

#endif