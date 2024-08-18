#include "front/semantic.h"

#include <cassert>

using ir::Function;
using ir::Instruction;
using ir::Operand;
using ir::Operator;

#define TODO assert(0 && "TODO");
#define COPY_EXP_NODE(from, to)              \
    to->is_computable = from->is_computable; \
    to->v             = from->v;             \
    to->t             = from->t;
#define CHILD_PTR(type, index) dynamic_cast<type*>(root->children[index])
#define GET_CHILD_PTR(node, type, index) \
    type* node = NULL;                   \
    if (CHILDREN_NUM(root) > index)      \
        node = dynamic_cast<type*>(root->children[index]);
#define CHILDREN_NUM(root) (int)root->children.size()
#define CHILD_PTR_R(type, index)                                                                \
    ({                                                                                          \
        int CHILDREN_NUM_MACRO = CHILDREN_NUM(root);                                            \
        dynamic_cast<type*>(root->children[(CHILDREN_NUM_MACRO + index) % CHILDREN_NUM_MACRO]); \
    })
#define GET_CHILD_PTR_R(node, type, index)                                                      \
    auto node = ({                                                                              \
        int CHILDREN_NUM_MACRO = CHILDREN_NUM(root);                                            \
        dynamic_cast<type*>(root->children[(CHILDREN_NUM_MACRO + index) % CHILDREN_NUM_MACRO]); \
    })
map<std::string, ir::Function*>* frontend::get_lib_funcs()
{
    static map<std::string, ir::Function*> lib_funcs = {
        {"getint", new Function("getint", Type::Int)},
        {"getch", new Function("getch", Type::Int)},
        {"getfloat", new Function("getfloat", Type::Float)},
        {"getarray", new Function("getarray", {Operand("arr", Type::IntPtr)}, Type::Int)},
        {"getfarray", new Function("getfarray", {Operand("arr", Type::FloatPtr)}, Type::Int)},
        {"putint", new Function("putint", {Operand("i", Type::Int)}, Type::null)},
        {"putch", new Function("putch", {Operand("i", Type::Int)}, Type::null)},
        {"putfloat", new Function("putfloat", {Operand("f", Type::Float)}, Type::null)},
        {"putarray",
         new Function("putarray", {Operand("n", Type::Int), Operand("arr", Type::IntPtr)},
                      Type::null)},
        {"putfarray",
         new Function("putfarray", {Operand("n", Type::Int), Operand("arr", Type::FloatPtr)},
                      Type::null)},
    };
    return &lib_funcs;
}

int Operand_uid = 0;
void frontend::SymbolTable::add_scope(Block* node)
{
    TODO;
}
void frontend::SymbolTable::exit_scope()
{
    TODO;
}

string frontend::SymbolTable::get_scoped_name(string id) const
{
    TODO;
}

Operand frontend::SymbolTable::get_operand(string id) const
{
    TODO;
}

enum class DataType { val,
                      literal,
                      ptr };
ir::Type Type2Type(ir::Type t, DataType op)
{
    switch (op) {
        case DataType::val:
            switch (t) {
                case ir::Type::Int:
                case ir::Type::IntLiteral:
                case ir::Type::IntPtr: return ir::Type::Int;
                case ir::Type::Float:
                case ir::Type::FloatLiteral:
                case ir::Type::FloatPtr: return ir::Type::Float;
            }
        case DataType::literal:
            switch (t) {
                case ir::Type::Int:
                case ir::Type::IntLiteral:
                case ir::Type::IntPtr: return ir::Type::IntLiteral;
                case ir::Type::Float:
                case ir::Type::FloatLiteral:
                case ir::Type::FloatPtr: return ir::Type::FloatLiteral;
            }
        case DataType::ptr:
            switch (t) {
                case ir::Type::Int:
                case ir::Type::IntLiteral:
                case ir::Type::IntPtr: return ir::Type::IntPtr;
                case ir::Type::Float:
                case ir::Type::FloatLiteral:
                case ir::Type::FloatPtr: return ir::Type::FloatPtr;
            }
    }
    return ir::Type::null;
}

frontend::STE frontend::SymbolTable::get_ste(string id) const
{
    for (int i = this->scope_stack.size() - 1; i >= 0; i--) {
        auto ste = this->scope_stack[i].table.find(id);
        if (ste != this->scope_stack[i].table.end())
            return ste->second;
    }
    return STE();
}

frontend::Analyzer::Analyzer()
    : tmp_cnt(0), symbol_table()
{
    ;
}

ir::Program frontend::Analyzer::get_ir_program(CompUnit* root)
{
    ir::Program program;
    ir::Function globalfunc;
    globalfunc.name = "MYinitfunc";
    program.addFunction(globalfunc);
    this->symbol_table.scope_stack.push_back(ScopeInfo());
    analysisCompUnit(root, program);
    program.functions[0].addInst(new ir::Instruction(  //
        ir::Operand(),                                 //
        ir::Operand(),                                 //
        ir::Operand(),                                 //
        ir::Operator::_return));
    for (auto i : this->symbol_table.scope_stack[0].table) {
        int size = 1;
        for (auto j : i.second.dimension)
            size = size * j;
        if (size == 1)
            program.globalVal.push_back(ir::GlobalVal(i.second.operand));
        else
            program.globalVal.push_back(ir::GlobalVal(i.second.operand, size));
    }
    return program;
}

void frontend::Analyzer::analysisCompUnit(CompUnit* root, ir::Program& program)
{
    GET_CHILD_PTR(p0, Decl, 0);
    if (p0 != NULL) {
        analysisDecl(p0, program.functions[0].InstVec);
    }
    else {
        GET_CHILD_PTR(p00, FuncDef, 0);
        assert(p00);
        ir::Function* func = new ir::Function;
        analysisFuncDef(p00, *func);
        program.addFunction(*func);
    }
    if (root->children.size() == 2)
        analysisCompUnit(CHILD_PTR(CompUnit, 1), program);
    return;
}
void frontend::Analyzer::analysisDecl(Decl* root, vector<ir::Instruction*>& instrs)
{
    GET_CHILD_PTR(p0, ConstDecl, 0);
    if (p0 != NULL) {
        analysisConstDecl(CHILD_PTR(ConstDecl, 0), instrs);
        return;
    }
    GET_CHILD_PTR(p00, VarDecl, 0);
    if (p00 != NULL)
        analysisVarDecl(CHILD_PTR(VarDecl, 0), instrs);
    return;
}
std::string namespa = "global";
void frontend::Analyzer::analysisFuncDef(FuncDef* root, ir::Function& func)
{
    this->tmp_cnt++;
    this->symbol_table.scope_stack.push_back(ScopeInfo());
    GET_CHILD_PTR(p0, FuncType, 0);
    analysisFuncType(p0);
    root->t = p0->t;
    GET_CHILD_PTR(p1, Term, 1);
    root->n = p1->token.value;
    if (root->n == "main") {
        func.addInst(new ir::CallInst(              //
            ir::Operand("MYinitfunc", Type::null),  //
            std::vector<Operand>(),                 //
            ir::Operand()));
    }
    GET_CHILD_PTR(p3, FuncFParams, 3);
    if (p3 != NULL) {
        analysisFuncFParams(p3, func);
    }
    func.name                               = root->n;
    func.returnType                         = root->t;
    func.ParameterList                      = (p3 != NULL) ? p3->ParameterList : vector<Operand>();
    namespa                                 = root->n;
    this->symbol_table.functions[func.name] = &func;
    analysisBlock(CHILD_PTR_R(Block, -1), func.InstVec);
    if (func.InstVec.back()->op != ir::Operator::_return) {
        func.InstVec.push_back(new ir::Instruction(  //
            ir::Operand(),                           //
            ir::Operand(),                           //
            ir::Operand(),                           //
            ir::Operator::_return));
    }
    this->symbol_table.scope_stack.pop_back();
    this->tmp_cnt--;
    return;
}
void frontend::Analyzer::analysisConstDecl(ConstDecl* root, vector<ir::Instruction*>& instrs)
{
    GET_CHILD_PTR(p1, BType, 1);
    analysisBType(p1);
    root->t = p1->t;
    for (int i = 2; i < CHILDREN_NUM(root); i = i + 2)
        analysisConstDef(CHILD_PTR(ConstDef, i), instrs);
    return;
}
void frontend::Analyzer::analysisConstDef(ConstDef* root, vector<ir::Instruction*>& instrs)
{
    STE ste;
    ir::Type t     = ((ConstDecl*)(root->parent))->t;
    root->arr_name = CHILD_PTR(Term, 0)->token.value + "_" + namespa;
    int alloc_size = 1;
    GET_CHILD_PTR_R(p0, ConstInitVal, -1);
    for (int i = 2; i < CHILDREN_NUM(root) - 1; i = i + 3) {
        GET_CHILD_PTR(p2, ConstExp, 2);
        analysisConstExp(p2, instrs);
        ste.dimension.push_back(std::stoi(p2->v.name));
        alloc_size *= std::stoi(p2->v.name);
    }
    if (ste.dimension.size() != 0) {  // const int a[2];
        ste.operand                                                         = ir::Operand(root->arr_name, (t == Type::Int) ? Type::IntPtr : Type::FloatPtr);
        this->symbol_table.scope_stack[this->tmp_cnt].table[root->arr_name] = ste;
        instrs.push_back(new ir::Instruction(                           //
            ir::Operand(std::to_string(alloc_size), Type::IntLiteral),  //
            ir::Operand(),                                              //
            ste.operand,                                                //
            ir::Operator::alloc));
        if (p0 != NULL) {
            analysisConstInitVal(p0, instrs);
            for (int j = 0; j < (int)p0->vecV.size(); j++)             // const int a[2] = {2,3};
                instrs.push_back(new ir::Instruction(                  //
                    ste.operand,                                       //
                    ir::Operand(std::to_string(j), Type::IntLiteral),  //
                    p0->vecV[j],                                       //
                    ir::Operator::store));
        }
    }
    else {
        analysisConstInitVal(p0, instrs);
        // const int a = 2;
        // LVal 'a' => '2'
        ste.operand = (t == Type::Int) ? p0->vecVar[0].v_int : p0->vecVar[0].v_float;
        //
        this->symbol_table.scope_stack[this->tmp_cnt].table[root->arr_name] = ste;
        // const int a = b;
        if (p0->vecV[0].type != ir::Type::IntLiteral &&
            p0->vecV[0].type != ir::Type::FloatLiteral) {
            instrs.push_back(new ir::Instruction(                                         //
                (t == Type::Int) ? p0->vecVar[0].v_int : p0->vecVar[0].v_float,           //
                ir::Operand(),                                                            //
                ir::Operand(root->arr_name, (t == Type::Int) ? Type::Int : Type::Float),  //
                (t == Type::Int) ? ir::Operator::def : ir::Operator::fdef));
        }
        return;
    }
}

void frontend::Analyzer::analysisConstExp(ConstExp* root, vector<ir::Instruction*>& instrs)
{
    GET_CHILD_PTR(p0, AddExp, 0);
    analysisAddExp(p0, instrs);
    root->is_computable = true;
    root->v             = p0->v;
    root->var           = p0->var;
    root->t             = p0->t;
    return;
}
void frontend::Analyzer::analysisConstInitVal(ConstInitVal* root, vector<ir::Instruction*>& instrs)
{
    GET_CHILD_PTR(p0, ConstExp, 0);
    if (p0 != NULL) {
        analysisConstExp(p0, instrs);
        root->vecV.push_back(p0->v);
        root->vecVar.push_back(p0->var);
        return;
    }
    for (int i = 1; i < CHILDREN_NUM(root); i = i + 2) {
        GET_CHILD_PTR(p00, ConstInitVal, i);
        analysisConstInitVal(p00, instrs);
        for (auto j : p00->vecV) {
            root->vecV.push_back(j);
        }
        for (auto j : p00->vecVar) {
            root->vecVar.push_back(j);
        }
    }
    return;
}
void frontend::Analyzer::analysisVarDecl(VarDecl* root, vector<ir::Instruction*>& instrs)
{
    GET_CHILD_PTR(p0, BType, 0);
    analysisBType(p0);
    root->t = p0->t;
    for (int i = 1; i < CHILDREN_NUM(root); i = i + 2) {
        analysisVarDef(CHILD_PTR(VarDef, i), instrs);
    }
    return;
}
void frontend::Analyzer::analysisVarDef(VarDef* root, vector<ir::Instruction*>& instrs)
{
    STE ste;
    ir::Type t     = ((VarDecl*)(root->parent))->t;
    bool is_float  = t == Type::Float;
    root->arr_name = CHILD_PTR(Term, 0)->token.value + "_" + namespa;
    int alloc_size = 1;
    for (int i = 2; i < CHILDREN_NUM(root) - 1; i = i + 3) {
        GET_CHILD_PTR(p2, ConstExp, i);
        analysisConstExp(p2, instrs);
        ste.dimension.push_back(std::stoi(p2->v.name));
        alloc_size *= std::stoi(p2->v.name);
    }
    GET_CHILD_PTR_R(p0, InitVal, -1);
    if (ste.dimension.size() != 0) {  // int a[2];
        ste.operand                                                         = ir::Operand(root->arr_name, is_float ? Type::FloatPtr : Type::IntPtr);
        this->symbol_table.scope_stack[this->tmp_cnt].table[root->arr_name] = ste;
        instrs.push_back(new ir::Instruction(                           //
            ir::Operand(std::to_string(alloc_size), Type::IntLiteral),  //
            ir::Operand(),                                              //
            ste.operand,                                                //
            ir::Operator::alloc));
        // for (int i = 0; i < alloc_size; ++i) {
        //     instrs.push_back(new ir::Instruction(                                    //
        //         ste.operand,                                                         //
        //         ir::Operand(std::to_string(i), Type::IntLiteral),                    //
        //         ir::Operand("0", is_float ? Type::FloatLiteral : Type::IntLiteral),  //
        //         ir::Operator::store));
        // }
        if (p0 != NULL) {
            analysisInitVal(p0, instrs);
            for (int j = 0; j < (int)p0->vecV.size(); j++) {  // int a[2] = {2,3};
                if (p0->vecV[j].type == Type::IntLiteral && is_float) {
                    instrs.push_back(new ir::Instruction(                   //
                        ste.operand,                                        //
                        ir::Operand(std::to_string(j), Type::IntLiteral),   //
                        ir::Operand(p0->vecV[j].name, Type::FloatLiteral),  //
                        ir::Operator::store));
                    continue;
                }
                instrs.push_back(new ir::Instruction(                  //
                    ste.operand,                                       //
                    ir::Operand(std::to_string(j), Type::IntLiteral),  //
                    p0->vecV[j],                                       //
                    ir::Operator::store));
            }
        }
        return;
    }
    else {
        ste.operand                                                         = ir::Operand(root->arr_name, is_float ? ir::Type::Float : ir::Type::Int);
        this->symbol_table.scope_stack[this->tmp_cnt].table[root->arr_name] = ste;
        if (p0 != NULL) {
            analysisInitVal(p0, instrs);
            if (p0->vecV[0].type == ir::Type::Int) {
                instrs.push_back(new ir::Instruction(                                         // int a = 2;
                    p0->vecV[0],                                                              //
                    ir::Operand(),                                                            //
                    ir::Operand(root->arr_name, is_float ? ir::Type::Float : ir::Type::Int),  //
                    is_float ? ir::Operator::cvt_i2f : ir::Operator::def));
            }
            if (p0->vecV[0].type == ir::Type::IntLiteral)
                instrs.push_back(new ir::Instruction(  // int a = 2;
                    ir::Operand(p0->vecV[0].name,
                                is_float ? ir::Type::FloatLiteral : ir::Type::IntLiteral),  //
                    ir::Operand(),                                                          //
                    ir::Operand(root->arr_name,
                                is_float ? ir::Type::Float : ir::Type::Int),  //
                    is_float ? ir::Operator::fdef : ir::Operator::def));
            if (p0->vecV[0].type == ir::Type::Float)
                instrs.push_back(new ir::Instruction(                                         // int a = 2;
                    p0->vecV[0],                                                              //
                    ir::Operand(),                                                            //
                    ir::Operand(root->arr_name, is_float ? ir::Type::Float : ir::Type::Int),  //
                    is_float ? ir::Operator::fdef : ir::Operator::cvt_f2i));
            if (p0->vecV[0].type == ir::Type::FloatLiteral)
                instrs.push_back(new ir::Instruction(  // int a = 2;
                    ir::Operand(p0->vecV[0].name,
                                is_float ? ir::Type::FloatLiteral : ir::Type::IntLiteral),  //
                    ir::Operand(),                                                          //
                    ir::Operand(root->arr_name,
                                is_float ? ir::Type::Float : ir::Type::Int),  //
                    is_float ? ir::Operator::fdef : ir::Operator::def));
            return;
        }
        // int a;
        instrs.push_back(new ir::Instruction(                                         //
            ir::Operand("0", is_float ? Type::FloatLiteral : Type::IntLiteral),       //
            ir::Operand(),                                                            //
            ir::Operand(root->arr_name, is_float ? ir::Type::Float : ir::Type::Int),  //
            is_float ? ir::Operator::fdef : ir::Operator::def));
        return;
    }
}
void frontend::Analyzer::analysisInitVal(InitVal* root, vector<ir::Instruction*>& instrs)
{
    GET_CHILD_PTR(p0, Exp, 0);
    if (p0 != NULL) {
        analysisExp(p0, instrs);
        root->vecV.push_back(p0->v);
        root->vecVar.push_back(p0->var);
        return;
    }
    if (CHILDREN_NUM(root) == 2)  // int a[5] = {};
        return;
    for (int i = 1; i < CHILDREN_NUM(root); i = i + 2) {
        GET_CHILD_PTR(p00, InitVal, i);
        analysisInitVal(p00, instrs);
        for (auto j : p00->vecV)
            root->vecV.push_back(j);
        for (auto j : p00->vecVar) {
            root->vecVar.push_back(j);
        }
    }
    return;
}
void frontend::Analyzer::analysisFuncFParam(FuncFParam* root, ir::Function& func)
{
    GET_CHILD_PTR(p0, BType, 0);
    analysisBType(p0);
    GET_CHILD_PTR(p1, Term, 1);
    root->Parameter = ir::Operand(           //
        p1->token.value,                     //
        (CHILDREN_NUM(root) == 2) ? p0->t :  //
            (p0->t == Type::Int) ? Type::IntPtr :
                                   //
                                   Type::FloatPtr);
    for (int i = 5; i < CHILDREN_NUM(root); i = i + 3) {
        analysisExp(CHILD_PTR(Exp, i), func.InstVec);
    }
    return;
}
void frontend::Analyzer::analysisFuncFParams(FuncFParams* root, ir::Function& func)
{
    auto table = &this->symbol_table.scope_stack[this->tmp_cnt].table;
    for (int i = 0; i < CHILDREN_NUM(root); i = i + 2) {
        GET_CHILD_PTR(p0, FuncFParam, i);
        analysisFuncFParam(p0, func);
        STE ste;
        ste.operand                  = p0->Parameter;
        (*table)[p0->Parameter.name] = ste;
        root->ParameterList.push_back(p0->Parameter);
    }
    return;
}
void frontend::Analyzer::analysisBlock(Block* root, vector<ir::Instruction*>& instrs)
{
    this->tmp_cnt++;
    this->symbol_table.scope_stack.push_back(ScopeInfo());
    for (int i = 1; i < CHILDREN_NUM(root) - 1; i = i + 1) {
        analysisBlockItem(CHILD_PTR(BlockItem, i), instrs);
    }
    this->symbol_table.scope_stack.pop_back();
    this->tmp_cnt--;
    return;
}
void frontend::Analyzer::analysisBlockItem(BlockItem* root, vector<ir::Instruction*>& instrs)
{
    GET_CHILD_PTR(p0, Decl, 0);
    if (p0 != NULL) {
        analysisDecl(p0, instrs);
        return;
    }
    analysisStmt(CHILD_PTR(Stmt, 0), instrs);
    return;
}

vector<int> pc_while_s;
vector<int> pc_while_e;
vector<int> pc_else_s;
vector<int> pc_if_e;
void frontend::Analyzer::analysisStmt(Stmt* root, vector<ir::Instruction*>& instrs)
{
    GET_CHILD_PTR(p0, Term, 0);
    GET_CHILD_PTR(p1, Exp, 1);
    GET_CHILD_PTR(p2, Cond, 2);
    ir::Instruction* branch  = new ir::Instruction(  //
        ir::Operand(),                              //
        ir::Operand(),                              //
        ir::Operand("0", Type::IntLiteral),         //
        ir::Operator::_goto);
    ir::Instruction* branch1 = new ir::Instruction(  //
        ir::Operand(),                               //
        ir::Operand(),                               //
        ir::Operand("0", Type::IntLiteral),          //
        ir::Operator::_goto);
    if (p0 != NULL)
        switch (p0->token.type) {
            case TokenType::IFTK:
                analysisCond(p2, instrs);
                instrs.push_back(new ir::Instruction(    //
                    p2->v,                               //
                    ir::Operand(),                       //
                    ir::Operand("2", Type::IntLiteral),  //
                    ir::Operator::_goto));
                pc_else_s.push_back(instrs.size());
                instrs.push_back(branch);
                analysisStmt(CHILD_PTR(Stmt, 4), instrs);
                pc_if_e.push_back(instrs.size());
                instrs.push_back(branch1);
                branch->des.name = std::to_string(instrs.size() - pc_else_s.back());
                if (CHILDREN_NUM(root) == 7) {
                    analysisStmt(CHILD_PTR(Stmt, 6), instrs);
                }
                branch1->des.name = std::to_string(instrs.size() - pc_if_e.back());
                pc_if_e.pop_back();
                pc_else_s.pop_back();
                return;
            case TokenType::WHILETK:
                pc_while_s.push_back(instrs.size());
                analysisCond(p2, instrs);
                instrs.push_back(new ir::Instruction(    //
                    p2->v,                               //
                    ir::Operand(),                       //
                    ir::Operand("2", Type::IntLiteral),  //
                    ir::Operator::_goto));
                pc_while_e.push_back(instrs.size());
                instrs.push_back(branch);
                analysisStmt(CHILD_PTR(Stmt, 4), instrs);
                instrs.push_back(new ir::Instruction(  //
                    ir::Operand(),                     //
                    ir::Operand(),                     //
                    ir::Operand("-" + std::to_string(instrs.size() - pc_while_s.back()),
                                Type::IntLiteral),  //
                    ir::Operator::_goto));
                branch->des.name = std::to_string(instrs.size() - pc_while_e.back());
                pc_while_s.pop_back();
                pc_while_e.pop_back();
                return;
            case TokenType::BREAKTK:
                instrs.push_back(new ir::Instruction(  //
                    ir::Operand(),                     //
                    ir::Operand(),                     //
                    ir::Operand("-" + std::to_string(instrs.size() - pc_while_e.back()),
                                Type::IntLiteral),  //
                    ir::Operator::_goto));
                return;
            case TokenType::CONTINUETK:
                instrs.push_back(new ir::Instruction(  //
                    ir::Operand(),                     //
                    ir::Operand(),                     //
                    ir::Operand("-" + std::to_string(instrs.size() - pc_while_s.back()),
                                Type::IntLiteral),  //
                    ir::Operator::_goto));
                return;
            case TokenType::RETURNTK:
                if (p1 != NULL) {
                    analysisExp(p1, instrs);
                    if (p1->v.type == ir::Type::Float &&
                        this->symbol_table.functions[namespa]->returnType == ir::Type::Int) {
                        instrs.push_back(new ir::Instruction(                    //
                            p1->v,                                               //
                            ir::Operand(),                                       //
                            ir::Operand(p1->v.name + "_cvtf2i", ir::Type::Int),  //
                            ir::Operator::cvt_f2i));
                        instrs.push_back(new ir::Instruction(                    //
                            ir::Operand(p1->v.name + "_cvtf2i", ir::Type::Int),  //
                            ir::Operand(),                                       //
                            ir::Operand(),                                       //
                            ir::Operator::_return));
                        return;
                    }
                    if (p1->v.type == ir::Type::FloatLiteral &&
                        this->symbol_table.functions[namespa]->returnType == ir::Type::Int) {
                        instrs.push_back(new ir::Instruction(               //
                            ir::Operand(p1->v.name, ir::Type::IntLiteral),  //
                            ir::Operand(),                                  //
                            ir::Operand(),                                  //
                            ir::Operator::_return));
                        return;
                    }
                    if (p1->v.type == ir::Type::Int &&
                        this->symbol_table.functions[namespa]->returnType == ir::Type::Float) {
                        instrs.push_back(new ir::Instruction(                      //
                            p1->v,                                                 //
                            ir::Operand(),                                         //
                            ir::Operand(p1->v.name + "_cvtf2i", ir::Type::Float),  //
                            ir::Operator::cvt_f2i));
                        instrs.push_back(new ir::Instruction(                      //
                            ir::Operand(p1->v.name + "_cvtf2i", ir::Type::Float),  //
                            ir::Operand(),                                         //
                            ir::Operand(),                                         //
                            ir::Operator::_return));
                        return;
                    }
                    if (p1->v.type == ir::Type::IntLiteral &&
                        this->symbol_table.functions[namespa]->returnType == ir::Type::Float) {
                        instrs.push_back(new ir::Instruction(                 //
                            ir::Operand(p1->v.name, ir::Type::FloatLiteral),  //
                            ir::Operand(),                                    //
                            ir::Operand(),                                    //
                            ir::Operator::_return));
                        return;
                    }
                    instrs.push_back(new ir::Instruction(  //
                        p1->v,                             //
                        ir::Operand(),                     //
                        ir::Operand(),                     //
                        ir::Operator::_return));
                    return;
                }
                return;
            default: break;
        }
    else {
        GET_CHILD_PTR(p3, LVal, 0);
        GET_CHILD_PTR(p4, Exp, 2);
        switch (root->children[0]->type) {
            case NodeType::LVAL:
                analysisLVal(p3, instrs);
                analysisExp(p4, instrs);
                if (p3->t == ir::Type::Int)
                    instrs.push_back(new ir::Instruction(  //
                        p4->v,                             //
                        ir::Operand(),                     //
                        p3->v,                             //
                        ir::Operator::mov));
                if (p3->t == ir::Type::IntPtr || p3->t == ir::Type::FloatPtr)
                    instrs.push_back(new ir::Instruction(  //
                        p3->v,                             //
                        p3->i,                             //
                        p4->v,                             //
                        ir::Operator::store));
                break;
            case NodeType::BLOCK: analysisBlock(CHILD_PTR(Block, 0), instrs); break;
            case NodeType::EXP: analysisExp(CHILD_PTR(Exp, 0), instrs); break;
            default: break;
        }
    }
    return;
}
void frontend::Analyzer::analysisExp(Exp* root, vector<ir::Instruction*>& instrs)
{
    GET_CHILD_PTR(p0, AddExp, 0);
    analysisAddExp(p0, instrs);
    root->t             = p0->t;
    root->v             = p0->v;
    root->var           = p0->var;
    root->is_computable = p0->is_computable;
    return;
}
void frontend::Analyzer::analysisCond(Cond* root, vector<ir::Instruction*>& instrs)
{
    GET_CHILD_PTR(p0, LOrExp, 0);
    analysisLOrExp(p0, instrs);
    root->t             = p0->t;
    root->v             = p0->v;
    root->var           = p0->var;
    root->is_computable = p0->is_computable;
    return;
}
void frontend::Analyzer::analysisLVal(LVal* root, vector<ir::Instruction*>& instrs)
{
    std::string name = CHILD_PTR(Term, 0)->token.value;
    STE ste          = this->symbol_table.get_ste(name + "_" + namespa);  // define in func
    if (ste.operand.type == ir::Type::null)
        ste = this->symbol_table.get_ste(name);  // define in func paramlist
    if (ste.operand.type == ir::Type::null)
        ste = this->symbol_table.get_ste(name + "_global");  // define global
    root->v = ste.operand;
    root->t = ste.operand.type;
    if (root->t == ir::Type::IntLiteral) {
        root->var.t     = ir::Type::IntLiteral;
        root->var.v_int = ste.operand;
    }
    if (root->t == ir::Type::FloatLiteral) {
        root->var.t       = ir::Type::FloatLiteral;
        root->var.v_float = ste.operand;
    }
    if (root->t == ir::Type::IntLiteral || root->t == ir::Type::FloatLiteral)
        root->is_computable = true;
    else
        root->is_computable = false;
    if (CHILDREN_NUM(root) == 1)
        return;
    // else A[n][m]
    root->i = ir::Operand(name + "_" + std::to_string(Operand_uid++), ir::Type::Int);
    instrs.push_back(new ir::Instruction(        //
        ir::Operand("0", ir::Type::IntLiteral),  //
        ir::Operand(),                           //
        root->i,                                 //
        ir::Operator::def));
    vector<int> index_weight = {1};
    if (ste.dimension.size() != 0) {
        for (auto i = ste.dimension.rbegin(); (i + 1) != ste.dimension.rend(); ++i) {
            index_weight.push_back((*i) * index_weight.back());
        }
    }
    // computable condition isn't considered!
    auto j = index_weight.rbegin();
    for (int i = 2; i < CHILDREN_NUM(root); i = i + 3) {
        GET_CHILD_PTR(p0, Exp, i);
        analysisExp(p0, instrs);
        ir::Operand LVal_arr_d("arr_d" + std::to_string((i - 2) / 3) + "i" + p0->v.name,
                               ir::Type::Int);
        instrs.push_back(new ir::Instruction(        //
            ir::Operand("0", ir::Type::IntLiteral),  //
            ir::Operand(),                           //
            LVal_arr_d,                              //
            ir::Operator::def));
        instrs.push_back(new ir::Instruction(                   //
            p0->v,                                              //
            ir::Operand(std::to_string(*j), Type::IntLiteral),  //
            LVal_arr_d,                                         //
            ir::Operator::mul));
        instrs.push_back(new ir::Instruction(  //
            LVal_arr_d,                        //
            root->i,                           //
            root->i,                           //
            ir::Operator::add));
        j++;
    }

    return;
}
void frontend::Analyzer::analysisPrimaryExp(PrimaryExp* root, vector<ir::Instruction*>& instrs)
{
    GET_CHILD_PTR(p0, Exp, 1);
    GET_CHILD_PTR(p1, LVal, 0);
    GET_CHILD_PTR(p2, Number, 0);
    switch (root->children[0]->type) {
        case NodeType::TERMINAL:  // (EXP)
            analysisExp(p0, instrs);
            root->t             = p0->t;
            root->v             = p0->v;
            root->var           = p0->var;
            root->is_computable = p0->is_computable;
            break;
        case NodeType::LVAL:
            analysisLVal(p1, instrs);
            root->v             = p1->v;
            root->var           = p1->var;
            root->t             = p1->t;
            root->is_computable = p1->is_computable;
            if (p1->t == ir::Type::IntPtr && p1->i.type != ir::Type::null) {
                root->v         = ir::Operand(p1->v.name + "_at_" + p1->i.name, ir::Type::Int);
                root->var.v_int = ir::Operand(p1->v.name + "_at_" + p1->i.name, ir::Type::Int);
                instrs.push_back(new ir::Instruction(  //
                    p1->v,                             //
                    p1->i,                             //
                    root->v,                           //
                    ir::Operator::load));
            }
            if (p1->t == ir::Type::FloatPtr && p1->i.type != ir::Type::null) {
                root->v           = ir::Operand(p1->v.name + "_at_" + p1->i.name, ir::Type::Float);
                root->var.v_float = ir::Operand(p1->v.name + "_at_" + p1->i.name, ir::Type::Float);
                instrs.push_back(new ir::Instruction(  //
                    p1->v,                             //
                    p1->i,                             //
                    root->v,                           //
                    ir::Operator::load));
            }
            break;
        case NodeType::NUMBER:
            analysisNumber(p2);
            root->t             = p2->t;
            root->v             = p2->v;
            root->var           = p2->var;
            root->is_computable = p2->is_computable;
            break;
        default: break;
    }
    return;
}
void frontend::Analyzer::analysisUnaryExp(UnaryExp* root, vector<ir::Instruction*>& instrs)
{
    GET_CHILD_PTR(p0, PrimaryExp, 0);
    GET_CHILD_PTR(p1, FuncRParams, 2);
    GET_CHILD_PTR(p2, UnaryExp, 1);
    GET_CHILD_PTR(p3, Term, 0);
    GET_CHILD_PTR(p4, UnaryOp, 0);
    std::vector<ir::Operand>::iterator param_in;
    std::string name                                = "MY_NULL";
    std::map<std::string, ir::Function*>* lib_func  = get_lib_funcs();
    std::map<std::string, ir::Function*>* user_func = &this->symbol_table.functions;
    std::map<std::string, ir::Function*>::iterator func_call;

    bool is_float = false;
    if (p3 != NULL) {
        name      = p3->token.value;
        func_call = lib_func->find(name);
        if (func_call == lib_func->cend()) {
            func_call = user_func->find(name);
        }
        assert(func_call != user_func->cend());
    }
    switch (root->children[0]->type) {
        case NodeType::PRIMARYEXP:
            analysisPrimaryExp(p0, instrs);
            root->t             = p0->t;
            root->v             = p0->v;
            root->var           = p0->var;
            root->is_computable = p0->is_computable;
            break;
        case NodeType::TERMINAL:
            if (p1 != NULL) {
                analysisFuncRParams(p1, instrs);
                param_in = p1->paraVec.begin();
            }
            root->v.type = (*func_call).second->returnType;
            root->var.t  = (*func_call).second->returnType;
            for (auto param : (*func_call).second->ParameterList) {
                if (param.type == ir::Type::Int && (*param_in).type == ir::Type::Float) {
                    instrs.push_back(new ir::Instruction(  //
                        *param_in,                         //
                        ir::Operand(),                     //
                        param,                             //
                        ir::Operator::cvt_f2i));
                    *param_in = param;
                }
                if (param.type == ir::Type::Float && (*param_in).type == ir::Type::Int) {
                    instrs.push_back(new ir::Instruction(  //
                        *param_in,                         //
                        ir::Operand(),                     //
                        param,                             //
                        ir::Operator::cvt_i2f));
                    *param_in = param;
                }
                if (param.type == ir::Type::Float && (*param_in).type == ir::Type::IntLiteral) {
                    instrs.push_back(new ir::Instruction(                       //
                        ir::Operand((*param_in).name, ir::Type::FloatLiteral),  //
                        ir::Operand(),                                          //
                        param,                                                  //
                        ir::Operator::fdef));
                    *param_in = param;
                }
                if (param.type == ir::Type::Int && (*param_in).type == ir::Type::FloatLiteral) {
                    instrs.push_back(new ir::Instruction(                     //
                        ir::Operand((*param_in).name, ir::Type::IntLiteral),  //
                        ir::Operand(),                                        //
                        param,                                                //
                        ir::Operator::def));
                    *param_in = param;
                }

                param_in++;
            }
            is_float            = root->v.type == ir::Type::Float;
            root->v.name        = name + "_ret" + std::to_string(Operand_uid++);
            root->var.v_int     = ir::Operand(root->v.name, ir::Type::Int);
            root->var.v_float   = ir::Operand(root->v.name, ir::Type::Float);
            root->is_computable = false;
            root->t             = root->v.type;
            if (root->v.type != ir::Type::null) {
                instrs.push_back(new ir::Instruction(                                            // int a = 2;
                    ir::Operand("0", is_float ? ir::Type::FloatLiteral : ir::Type::IntLiteral),  //
                    ir::Operand(),                                                               //
                    root->v,                                                                     //
                    is_float ? ir::Operator::fdef : ir::Operator::def));
            }
            instrs.push_back(new ir::CallInst(                       //
                ir::Operand(name, root->v.type),                     //
                (p1 != NULL) ? p1->paraVec : vector<ir::Operand>(),  //
                root->v));
            break;
        case NodeType::UNARYOP:
            analysisUnaryExp(p2, instrs);
            is_float = (p2->t == ir::Type::Float || p2->t == ir::Type::FloatLiteral);
            analysisUnaryOp(p4);
            if (p4->tk_t == TokenType::MINU) {
                if (p2->is_computable) {
                    root->is_computable = true;
                    root->t             = p2->t;
                    if (p2->v.name[0] == '-') {
                        root->v           = Operand(                                    //
                            std::to_string(std::stoi(p2->v.name.substr(1))),  //
                            Type2Type(p2->t, DataType::literal));
                        root->var.v_int   = Operand(std::to_string(std::stoi(p2->v.name.substr(1))),
                                                    Type::IntLiteral);
                        root->var.v_float = Operand(std::to_string(std::stoi(p2->v.name.substr(1))),
                                                    Type::FloatLiteral);
                        return;
                    }
                    else {
                        root->v = Operand(                                //
                            "-" + std::to_string(std::stoi(p2->v.name)),  //
                            Type2Type(p2->t, DataType::literal));
                        root->var.v_int =
                            Operand("-" + std::to_string(std::stoi(p2->v.name)), Type::IntLiteral);
                        root->var.v_float = Operand("-" + std::to_string(std::stoi(p2->v.name)),
                                                    Type::FloatLiteral);
                        return;
                    }
                }
                else {
                    std::string name    = "Unary_" + std::to_string(Operand_uid++);
                    root->v             = ir::Operand(name, is_float ? ir::Type::Float : ir::Type::Int);
                    root->var.v_int     = Operand(name, Type::Int);
                    root->var.v_float   = Operand(name, Type::Float);
                    root->t             = ir::Type::Int;
                    root->is_computable = false;
                    instrs.push_back(new ir::Instruction(  //
                        ir::Operand("0",
                                    is_float ? ir::Type::FloatLiteral : ir::Type::IntLiteral),  //
                        ir::Operand(),                                                          //
                        ir::Operand("ZERO", is_float ? ir::Type::Float : ir::Type::Int),        //
                        is_float ? ir::Operator::fdef : ir::Operator::def));
                    instrs.push_back(new ir::Instruction(                                 //
                        ir::Operand("ZERO", is_float ? ir::Type::Float : ir::Type::Int),  //
                        p2->v,                                                            //
                        root->v,                                                          //
                        is_float ? ir::Operator::fsub : ir::Operator::sub));
                }
                return;
            }

            if (p4->tk_t == TokenType::NOT) {
                if (p2->is_computable) {
                    root->t = ir::Type::IntLiteral;
                    if (p2->v.name != "0")
                        root->v = ir::Operand("0", ir::Type::IntLiteral);
                    else
                        root->v = ir::Operand("1", ir::Type::IntLiteral);
                }
                else {
                    root->v = ir::Operand("Unary_" + std::to_string(Operand_uid++), ir::Type::Int);
                    root->t = ir::Type::Int;
                    instrs.push_back(new ir::Instruction(  //
                        p2->v,                             //
                        ir::Operand(),                     //
                        root->v,                           //
                        ir::Operator::_not));
                }
                return;
            }
            root->t             = p2->t;
            root->v             = p2->v;
            root->is_computable = p2->is_computable;
            return;
        default: return;
    }
    return;
}
void frontend::Analyzer::analysisFuncRParams(FuncRParams* root, vector<ir::Instruction*>& instrs)
{
    for (int i = 0; i < CHILDREN_NUM(root); i = i + 2) {
        GET_CHILD_PTR(p0, Exp, i);
        analysisExp(p0, instrs);
        root->paraVec.push_back(p0->v);
    }
    return;
}

void frontend::Analyzer::analysisMulExp(MulExp* root, vector<ir::Instruction*>& instrs)
{
    int size = CHILDREN_NUM(root);
    GET_CHILD_PTR(p0, UnaryExp, 0);
    analysisUnaryExp(p0, instrs);
    if (CHILDREN_NUM(root) == 1) {
        root->v             = p0->v;
        root->is_computable = p0->is_computable;
        root->t             = p0->t;
        root->var           = p0->var;
        return;
    }
    bool is_computable = p0->is_computable;
    bool is_float      = p0->t == ir::Type::FloatLiteral || p0->t == ir::Type::Float;
    for (int i = 2; i < size; i = i + 2) {
        GET_CHILD_PTR(p1, UnaryExp, i);
        analysisUnaryExp(p1, instrs);
        is_computable = is_computable && p1->is_computable;
        is_float      = is_float || (p1->t == ir::Type::FloatLiteral || p1->t == ir::Type::Float);
    }
    root->is_computable = is_computable;
    if (is_computable) {
        if (!is_float) {
            int mul = std::stoi(p0->v.name);
            for (int i = 2; i < size; i = i + 2) {
                GET_CHILD_PTR(p1, UnaryExp, i);
                TokenType op = CHILD_PTR(Term, i - 1)->token.type;
                int num      = std::stoi(p1->v.name);
                if (op == TokenType::MULT)
                    mul *= num;
                if (op == TokenType::DIV)
                    mul /= num;
                if (op == TokenType::MOD)
                    mul %= num;
            }
            root->v         = ir::Operand(std::to_string(mul), ir::Type::IntLiteral);
            root->t         = ir::Type::IntLiteral;
            root->var.v_int = ir::Operand(std::to_string(mul), ir::Type::IntLiteral);
            root->var.t     = ir::Type::IntLiteral;
        }
        else {
            float mul = std::stof(p0->v.name);
            for (int i = 2; i < size; i = i + 2) {
                GET_CHILD_PTR(p1, UnaryExp, i);
                if (p1->is_computable) {
                    TokenType op = CHILD_PTR(Term, i - 1)->token.type;
                    float num    = std::stof(p1->v.name);
                    if (op == TokenType::MULT)
                        mul *= num;
                    if (op == TokenType::DIV)
                        mul /= num;
                    if (op == TokenType::MOD)
                        assert(0 && "float mod");
                }
            }
            root->v           = ir::Operand(std::to_string(mul), ir::Type::FloatLiteral);
            root->t           = ir::Type::FloatLiteral;
            root->var.v_float = ir::Operand(std::to_string(mul), ir::Type::FloatLiteral);
            root->var.t       = ir::Type::FloatLiteral;
        }
        return;
    }
    else {
        if (!is_float) {
            std::string name = "Mul_" + std::to_string(Operand_uid++);
            root->v          = ir::Operand(name, ir::Type::Int);
            root->t          = ir::Type::Int;
            root->var.v_int  = ir::Operand(name, ir::Type::Int);
            root->var.t      = ir::Type::Int;
            instrs.push_back(new ir::Instruction(        //
                ir::Operand("1", ir::Type::IntLiteral),  //
                ir::Operand(),                           //
                root->v,                                 //
                ir::Operator::def));
            ir::Operand op2 = p0->v;
            if (p0->is_computable) {
                op2 = ir::Operand("Mul_op2_" + std::to_string(Operand_uid++), ir::Type::Int);
                instrs.push_back(new ir::Instruction(  //
                    p0->v,                             //
                    ir::Operand(),                     //
                    op2,                               //
                    ir::Operator::def));
            }
            instrs.push_back(new ir::Instruction(  //
                root->v,                           //
                op2,                               //
                root->v,                           //
                ir::Operator::mul));
            for (int i = 2; i < size; i = i + 2) {
                TokenType op = CHILD_PTR(Term, i - 1)->token.type;
                GET_CHILD_PTR(p2, UnaryExp, i);
                op2 = p2->v;
                if (p2->is_computable) {
                    op2 = ir::Operand("Mul_op2_" + std::to_string(Operand_uid++), ir::Type::Int);
                    instrs.push_back(new ir::Instruction(  //
                        p2->v,                             //
                        ir::Operand(),                     //
                        op2,                               //
                        ir::Operator::def));
                }
                instrs.push_back(new ir::Instruction(              //
                    root->v,                                       //
                    op2,                                           //
                    root->v,                                       //
                    (op == TokenType::MULT) ? ir::Operator::mul :  //
                        (op == TokenType::DIV) ? ir::Operator::div :
                                                 //
                        (op == TokenType::MOD) ? ir::Operator::mod :
                                                 //
                                                 ir::Operator::mul));
            }
            return;
        }
        else {
            std::string name  = "FMul_" + std::to_string(Operand_uid++);
            root->v           = ir::Operand(name, ir::Type::Float);
            root->t           = ir::Type::Float;
            root->var.v_float = ir::Operand(name, ir::Type::Float);
            root->var.t       = ir::Type::Float;
            instrs.push_back(new ir::Instruction(          //
                ir::Operand("1", ir::Type::FloatLiteral),  //
                ir::Operand(),                             //
                root->v,                                   //
                ir::Operator::fdef));
            // deal with const float mul
            ir::Operand op2 = p0->v;
            if (p0->is_computable) {
                op2 = ir::Operand("FMul_op2_" + std::to_string(Operand_uid++), ir::Type::Float);
                instrs.push_back(new ir::Instruction(                 //
                    ir::Operand(p0->v.name, ir::Type::FloatLiteral),  //
                    ir::Operand(),                                    //
                    op2,                                              //
                    ir::Operator::fdef));
            }
            if (p0->t == ir::Type::Int) {
                op2 = ir::Operand(p0->v.name + "_cvti2f", ir::Type::Float);
                instrs.push_back(new ir::Instruction(  //
                    p0->v,                             //
                    ir::Operand(),                     //
                    op2,                               //
                    ir::Operator::cvt_i2f));
            }
            instrs.push_back(new ir::Instruction(  //
                root->v,                           //
                op2,                               //
                root->v,                           //
                ir::Operator::fmul));
            for (int i = 2; i < size; i = i + 2) {
                TokenType op = CHILD_PTR(Term, i - 1)->token.type;
                GET_CHILD_PTR(p2, UnaryExp, i);
                op2 = p2->v;
                if (p2->is_computable) {
                    op2 = ir::Operand("FMul_op2_" + std::to_string(Operand_uid++), ir::Type::Float);
                    instrs.push_back(new ir::Instruction(                 //
                        ir::Operand(p2->v.name, ir::Type::FloatLiteral),  //
                        ir::Operand(),                                    //
                        op2,                                              //
                        ir::Operator::fdef));
                }
                if (p2->t == ir::Type::Int) {
                    op2 = ir::Operand(p2->v.name + "_cvti2f", ir::Type::Float);
                    instrs.push_back(new ir::Instruction(  //
                        p2->v,                             //
                        ir::Operand(),                     //
                        op2,                               //
                        ir::Operator::cvt_i2f));
                }
                instrs.push_back(new ir::Instruction(               //
                    root->v,                                        //
                    op2,                                            //
                    root->v,                                        //
                    (op == TokenType::MULT) ? ir::Operator::fmul :  //
                        (op == TokenType::DIV) ? ir::Operator::fdiv :
                                                 //
                                                 ir::Operator::fmul));
            }
            return;
        }
        return;
    }
}
void frontend::Analyzer::analysisAddExp(AddExp* root, vector<ir::Instruction*>& instrs)
{
    int size = CHILDREN_NUM(root);
    GET_CHILD_PTR(p0, MulExp, 0);
    analysisMulExp(p0, instrs);
    if (CHILDREN_NUM(root) == 1) {
        root->v             = p0->v;
        root->var           = p0->var;
        root->is_computable = p0->is_computable;
        root->t             = p0->t;
        return;
    }
    bool is_computable = p0->is_computable;
    bool is_float      = p0->t == ir::Type::FloatLiteral || p0->t == ir::Type::Float;
    for (int i = 2; i < size; i = i + 2) {
        GET_CHILD_PTR(p1, MulExp, i);
        analysisMulExp(CHILD_PTR(MulExp, i), instrs);
        is_computable = is_computable && p1->is_computable;
        is_float      = is_float || p1->t == ir::Type::FloatLiteral || p1->t == ir::Type::Float;
    }
    // sum all numbers that can be calculated
    root->is_computable = is_computable;
    if (is_computable) {
        if (!is_float) {
            int sum = std::stoi(p0->v.name);
            for (int i = 2; i < size; i = i + 2) {
                TokenType tmp = CHILD_PTR(Term, i - 1)->token.type;
                if (tmp == TokenType::PLUS)
                    sum += std::stoi(CHILD_PTR(MulExp, i)->v.name);
                if (tmp == TokenType::MINU)
                    sum -= std::stoi(CHILD_PTR(MulExp, i)->v.name);
            }
            root->v         = ir::Operand(std::to_string(sum), ir::Type::IntLiteral);
            root->t         = ir::Type::IntLiteral;
            root->var.v_int = ir::Operand(std::to_string(sum), ir::Type::IntLiteral);
            root->var.t     = ir::Type::IntLiteral;
            return;
        }
        else {
            float sum = std::stof(p0->v.name);
            for (int i = 2; i < size; i = i + 2) {
                TokenType tmp = CHILD_PTR(Term, i - 1)->token.type;
                if (tmp == TokenType::PLUS)
                    sum += std::stof(CHILD_PTR(MulExp, i)->v.name);
                if (tmp == TokenType::MINU)
                    sum -= std::stof(CHILD_PTR(MulExp, i)->v.name);
            }
            root->v = ir::Operand(std::to_string(sum), ir::Type::FloatLiteral);
            root->t = ir::Type::FloatLiteral;
            return;
        }
    }
    else {
        if (!is_float) {
            root->v = ir::Operand("ADD_" + std::to_string(Operand_uid++), ir::Type::Int);
            root->t = ir::Type::Int;
            instrs.push_back(new ir::Instruction(        //
                ir::Operand("0", ir::Type::IntLiteral),  //
                ir::Operand(),                           //
                root->v,                                 //
                ir::Operator::def));
            instrs.push_back(new ir::Instruction(  //
                root->v,                           //
                p0->v,                             //
                root->v,                           //
                ir::Operator::add));
            for (int i = 2; i < size; i = i + 2) {
                TokenType tmp = CHILD_PTR(Term, i - 1)->token.type;
                GET_CHILD_PTR(p2, MulExp, i);
                if (p2->var.t == ir::Type::IntLiteral)
                    instrs.push_back(new ir::Instruction(  //
                        root->v,                           //
                        p2->v,                             //
                        root->v,                           //
                        (tmp == TokenType::PLUS) ? ir::Operator::addi :
                        (tmp == TokenType::MINU) ? ir::Operator::subi :
                                                   ir::Operator::subi));
                else
                    instrs.push_back(new ir::Instruction(  //
                        root->v,                           //
                        p2->v,                             //
                        root->v,                           //
                        (tmp == TokenType::PLUS) ? ir::Operator::add :
                        (tmp == TokenType::MINU) ? ir::Operator::sub :
                                                   ir::Operator::sub));
            }
            return;
        }
        else {
            root->v = ir::Operand("ADD_" + std::to_string(Operand_uid++), ir::Type::Float);
            root->t = ir::Type::Float;
            instrs.push_back(new ir::Instruction(          //
                ir::Operand("0", ir::Type::FloatLiteral),  //
                ir::Operand(),                             //
                root->v,                                   //
                ir::Operator::fdef));
            instrs.push_back(new ir::Instruction(  //
                root->v,                           //
                p0->v,                             //
                root->v,                           //
                ir::Operator::fadd));
            for (int i = 2; i < size; i = i + 2) {
                TokenType tmp = CHILD_PTR(Term, i - 1)->token.type;
                instrs.push_back(new ir::Instruction(  //
                    root->v,                           //
                    CHILD_PTR(MulExp, i)->v,           //
                    root->v,                           //
                    (tmp == TokenType::PLUS) ? ir::Operator::fadd :
                    (tmp == TokenType::MINU) ? ir::Operator::fsub :
                                               ir::Operator::fadd));
            }
            return;
        }
    }
}
void frontend::Analyzer::analysisRelExp(RelExp* root, vector<ir::Instruction*>& instrs)
{
    GET_CHILD_PTR(p0, AddExp, 0);
    analysisAddExp(p0, instrs);
    if (CHILDREN_NUM(root) == 1) {
        root->v             = p0->v;
        root->is_computable = p0->is_computable;
        root->t             = p0->t;
        return;
    }
    int size           = CHILDREN_NUM(root);
    bool is_computable = p0->is_computable;
    bool is_float      = p0->t == ir::Type::FloatLiteral || p0->t == ir::Type::Float;
    for (int i = 2; i < size; i = i + 2) {
        GET_CHILD_PTR(p1, AddExp, i);
        analysisAddExp(p1, instrs);
        is_computable = is_computable && p1->is_computable;
        is_float      = is_float || p0->t == ir::Type::FloatLiteral || p0->t == ir::Type::Float;
    }
    root->is_computable = is_computable;
    if (is_computable) {
        if (!is_float) {
            int ret = std::stoi(p0->v.name);
            for (int i = 2; i < size && is_computable; i = i + 2) {
                GET_CHILD_PTR(p1, AddExp, i);
                TokenType op = CHILD_PTR(Term, i - 1)->token.type;
                if (is_computable) {
                    if (op == TokenType::GEQ)
                        ret = (ret >= std::stoi(p1->v.name));
                    if (op == TokenType::LEQ)
                        ret = (ret <= std::stoi(p1->v.name));
                    if (op == TokenType::GTR)
                        ret = (ret > std::stoi(p1->v.name));
                    if (op == TokenType::LSS)
                        ret = (ret < std::stoi(p1->v.name));
                }
            }
            root->v = Operand(std::to_string(ret), ir::Type::IntLiteral);
            root->t = ir::Type::IntLiteral;
            return;
        }
        else {
            float ret = std::stof(p0->v.name);
            for (int i = 2; i < size && is_computable; i = i + 2) {
                GET_CHILD_PTR(p1, AddExp, i);
                TokenType op = CHILD_PTR(Term, i - 1)->token.type;
                if (is_computable) {
                    if (op == TokenType::GEQ)
                        ret = (ret >= std::stof(p1->v.name));
                    if (op == TokenType::LEQ)
                        ret = (ret <= std::stof(p1->v.name));
                    if (op == TokenType::GTR)
                        ret = (ret > std::stof(p1->v.name));
                    if (op == TokenType::LSS)
                        ret = (ret < std::stof(p1->v.name));
                }
            }
            root->v = Operand(std::to_string(ret), ir::Type::IntLiteral);
            root->t = ir::Type::IntLiteral;
            return;
        }
    }
    else {
        if (!is_float) {
            root->v = Operand("Rel_" + std::to_string(Operand_uid++), ir::Type::Int);
            root->t = ir::Type::Int;
            instrs.push_back(new ir::Instruction(  //
                p0->v,                             //
                ir::Operand(),                     //
                root->v,                           //
                ir::Operator::def));
            for (int i = 2; i < CHILDREN_NUM(root); i = i + 2) {
                GET_CHILD_PTR(p1, AddExp, i);
                TokenType tmp = CHILD_PTR(Term, i - 1)->token.type;
                instrs.push_back(new ir::Instruction(  // op_type == 0 neq root root p1
                    root->v,                           // op_type == 1 neq p1 root p1
                    p1->v,                             //
                    root->v,                           //
                    (tmp == TokenType::GEQ) ? ir::Operator::geq :
                    (tmp == TokenType::LEQ) ? ir::Operator::leq :
                    (tmp == TokenType::GTR) ? ir::Operator::gtr :
                    (tmp == TokenType::LSS) ? ir::Operator::lss :
                                              ir::Operator::geq));
            }
            return;
        }
        else {
            root->v = Operand("Rel_" + std::to_string(Operand_uid++), ir::Type::Float);
            root->t = ir::Type::Float;
            instrs.push_back(new ir::Instruction(  //
                p0->v,                             //
                ir::Operand(),                     //
                root->v,                           //
                ir::Operator::fdef));
            for (int i = 2; i < CHILDREN_NUM(root); i = i + 2) {
                GET_CHILD_PTR(p1, AddExp, i);
                TokenType tmp = CHILD_PTR(Term, i - 1)->token.type;
                instrs.push_back(new ir::Instruction(  // op_type == 0 neq root root p1
                    root->v,                           // op_type == 1 neq p1 root p1
                    ir::Operand(p1->v.name, (p1->v.type == ir::Type::IntLiteral) ?
                                                ir::Type::FloatLiteral :
                                                p1->v.type),  //
                    root->v,                                  //
                    (tmp == TokenType::GEQ) ? ir::Operator::fgeq :
                    (tmp == TokenType::LEQ) ? ir::Operator::fleq :
                    (tmp == TokenType::GTR) ? ir::Operator::fgtr :
                    (tmp == TokenType::LSS) ? ir::Operator::flss :
                                              ir::Operator::fgeq));
            }
            return;
        }
    }
}
void frontend::Analyzer::analysisEqExp(EqExp* root, vector<ir::Instruction*>& instrs)
{
    GET_CHILD_PTR(p0, RelExp, 0);
    analysisRelExp(p0, instrs);
    if (CHILDREN_NUM(root) == 1) {
        root->v             = p0->v;
        root->is_computable = p0->is_computable;
        root->t             = p0->t;
        return;
    }
    int size = CHILDREN_NUM(root);
    for (int i = 2; i < size; i = i + 2) {
        analysisRelExp(CHILD_PTR(RelExp, i), instrs);
    }
    bool is_computable = p0->is_computable;
    int ret            = is_computable ? std::stoi(p0->v.name) : 1;
    for (int i = 2; i < size && is_computable; i = i + 2) {
        GET_CHILD_PTR(p1, RelExp, i);
        is_computable = is_computable && p1->is_computable;
        TokenType op  = CHILD_PTR(Term, i - 1)->token.type;
        if (is_computable) {
            if (op == TokenType::EQL)
                ret = (ret == std::stoi(p1->v.name));
            if (op == TokenType::NEQ)
                ret = (ret != std::stoi(p1->v.name));
        }
    }
    if (is_computable) {
        root->v             = Operand(std::to_string(ret), ir::Type::IntLiteral);
        root->is_computable = true;
        root->t             = ir::Type::IntLiteral;
        return;
    }
    root->v             = Operand("Eq" + std::to_string(Operand_uid++), ir::Type::Int);
    root->t             = ir::Type::Int;
    root->is_computable = false;
    if (p0->is_computable)
        instrs.push_back(new ir::Instruction(  //
            p0->v,                             //
            ir::Operand(),                     //
            root->v,                           //
            ir::Operator::def));
    else {
        instrs.push_back(new ir::Instruction(        //
            ir::Operand("0", ir::Type::IntLiteral),  //
            ir::Operand(),                           //
            root->v,                                 //
            ir::Operator::def));
        instrs.push_back(new ir::Instruction(  //
            p0->v,                             //
            ir::Operand(),                     //
            root->v,                           //
            ir::Operator::mov));
    }
    for (int i = 2; i < CHILDREN_NUM(root); i = i + 2) {
        GET_CHILD_PTR(p1, RelExp, i);
        TokenType tmp = CHILD_PTR(Term, i - 1)->token.type;
        if (tmp == TokenType::NEQ)
            instrs.push_back(new ir::Instruction(  // op_type == 0 neq root root p1
                root->v,                           // op_type == 1 neq p1 root p1
                p1->v,                             //
                root->v,                           //
                ir::Operator::neq));
        if (tmp == TokenType::EQL)
            instrs.push_back(new ir::Instruction(  // op_type == 0 eq root root p1
                root->v,                           // op_type == 1 eq p1 root p1
                p1->v,                             //
                root->v,                           //
                ir::Operator::eq));
    }
    return;
}
vector<int> pc_LAnd_s;
void frontend::Analyzer::analysisLAndExp(LAndExp* root, vector<ir::Instruction*>& instrs)
{
    GET_CHILD_PTR(p0, EqExp, 0);
    analysisEqExp(p0, instrs);
    if (CHILDREN_NUM(root) == 1 ||
        (p0->is_computable && (p0->v.name == "0" || p0->v.name == ".0"))) {
        root->v             = p0->v;
        root->is_computable = p0->is_computable;
        root->t             = p0->t;
        return;
    }
    if (p0->is_computable && p0->v.name == "1") {
        GET_CHILD_PTR(p1, LAndExp, 2);
        analysisLAndExp(p1, instrs);
        root->v             = p1->v;
        root->is_computable = p1->is_computable;
        root->t             = p1->t;
        return;
    }
    root->v             = Operand("LAnd" + std::to_string(Operand_uid++), ir::Type::Int);
    root->t             = Type::Int;
    root->is_computable = false;
    instrs.push_back(new ir::Instruction(        // op_type == 0 and root root p1
        ir::Operand("0", ir::Type::IntLiteral),  // op_type == 1 and p1 root p1
        ir::Operand(),                           //
        root->v,                                 //
        ir::Operator::def));
    ir::Instruction* branch = new ir::Instruction(  //
        ir::Operand(),                              //
        ir::Operand(),                              //
        ir::Operand("0", Type::IntLiteral),         //
        ir::Operator::_goto);
    instrs.push_back(new ir::Instruction(    //
        p0->v,                               //
        ir::Operand(),                       //
        ir::Operand("2", Type::IntLiteral),  //
        ir::Operator::_goto));
    pc_LAnd_s.push_back(instrs.size());
    instrs.push_back(branch);
    GET_CHILD_PTR(p1, LAndExp, 2);
    analysisLAndExp(p1, instrs);
    if (p1->is_computable && p1->v.name == "0") {
        root->v             = ir::Operand("0", ir::Type::IntLiteral);
        root->is_computable = true;
        root->t             = ir::Type::IntLiteral;
        return;
    }
    if (p0->is_computable && p1->is_computable) {
        bool A              = (p0->v.name != "0");
        bool B              = (p1->v.name != "0");
        root->v             = ir::Operand((A && B) ? "1" : "0", ir::Type::IntLiteral);
        root->is_computable = true;
        root->t             = ir::Type::IntLiteral;
        return;
    }
    instrs.push_back(new ir::Instruction(  // op_type == 0 and root root p1
        p0->v,                             // op_type == 1 and p1 root p1
        p1->v,                             //
        root->v,                           //
        ir::Operator::_and));
    branch->des.name = std::to_string(instrs.size() - pc_LAnd_s.back());
    pc_LAnd_s.pop_back();
    return;
}
vector<int> pc_LOr_s;
void frontend::Analyzer::analysisLOrExp(LOrExp* root, vector<ir::Instruction*>& instrs)
{
    GET_CHILD_PTR(p0, LAndExp, 0);
    analysisLAndExp(p0, instrs);
    if (CHILDREN_NUM(root) == 1 || (p0->is_computable && p0->v.name == "1")) {
        root->v             = p0->v;
        root->is_computable = p0->is_computable;
        root->t             = p0->t;
        return;
    }
    if (p0->is_computable && p0->v.name == "0") {
        GET_CHILD_PTR(p1, LOrExp, 2);
        analysisLOrExp(p1, instrs);
        root->v             = p1->v;
        root->is_computable = p1->is_computable;
        root->t             = p1->t;
        return;
    }
    root->v             = Operand("LOr" + std::to_string(Operand_uid++), ir::Type::Int);
    root->t             = Type::Int;
    root->is_computable = false;
    instrs.push_back(new ir::Instruction(        // op_type == 0 and root root p1
        ir::Operand("1", ir::Type::IntLiteral),  // op_type == 1 and p1 root p1
        ir::Operand(),                           //
        root->v,                                 //
        ir::Operator::def));
    ir::Instruction* branch = new ir::Instruction(  //
        p0->v,                                      //
        ir::Operand(),                              //
        ir::Operand("0", Type::IntLiteral),         //
        ir::Operator::_goto);
    pc_LOr_s.push_back(instrs.size());
    instrs.push_back(branch);
    GET_CHILD_PTR(p1, LOrExp, 2);
    analysisLOrExp(p1, instrs);
    if (p1->is_computable && p1->v.name == "1") {
        root->v             = ir::Operand("1", ir::Type::IntLiteral);
        root->is_computable = true;
        root->t             = ir::Type::IntLiteral;
        return;
    }
    if (p0->is_computable && p1->is_computable) {
        bool A              = (p0->v.name != "0");
        bool B              = (p1->v.name != "0");
        root->v             = ir::Operand((A || B) ? "1" : "0", ir::Type::IntLiteral);
        root->is_computable = true;
        root->t             = ir::Type::IntLiteral;
        return;
    }
    instrs.push_back(new ir::Instruction(  // op_type == 0 and root root p1
        p0->v,                             // op_type == 1 and p1 root p1
        p1->v,                             //
        root->v,                           //
        ir::Operator::_or));
    branch->des.name = std::to_string(instrs.size() - pc_LOr_s.back());
    pc_LOr_s.pop_back();
    return;
}
void frontend::Analyzer::analysisBType(BType* root)
{
    GET_CHILD_PTR(p0, Term, 0);
    root->t = (p0->token.type == TokenType::INTTK) ? Type::Int : Type::Float;
    return;
}
void frontend::Analyzer::analysisFuncType(FuncType* root)
{
    GET_CHILD_PTR(p0, Term, 0);
    root->t = (p0->token.type == TokenType::INTTK) ? Type::Int : Type::null;
    root->t = (p0->token.type == TokenType::FLOATTK) ? Type::Float : root->t;
    return;
}
void frontend::Analyzer::analysisNumber(Number* root)
{
    GET_CHILD_PTR(p0, Term, 0);
    root->t             = (p0->token.type == TokenType::INTLTR) ? Type::IntLiteral : Type::FloatLiteral;
    root->is_computable = true;
    std::string number  = p0->token.value;
    int num             = 0;
    if (number.size() != 1 && root->t == Type::IntLiteral) {
        if (number[0] == '0' && number[1] != 'x') {  // deal with OCT
            for (auto i : number.substr(1)) {
                num = num * 8;
                num = num + i - '0';
            }
            number = std::to_string(num);
        }
        if (number[0] == '0' && number[1] == 'x') {  // deal with HEX
            for (auto i : number.substr(2)) {
                num      = num * 16;
                int tmp0 = i - '0';
                int tmp1 = i - 'a' + 10;
                num      = num + ((tmp0 > 9) ? tmp1 : tmp0);
            }
            number = std::to_string(num);
        }
    }
    root->v           = ir::Operand(number, root->t);
    root->var.t       = root->t;
    root->var.v_int   = (p0->token.type == TokenType::INTTK) ?
                            ir::Operand(number, ir::Type::IntLiteral) :
                            ir::Operand(number.substr(0, number.find('.')), ir::Type::IntLiteral);
    root->var.v_float = ir::Operand(number, ir::Type::FloatLiteral);
    return;
}
void frontend::Analyzer::analysisUnaryOp(UnaryOp* root)
{
    root->tk_t = CHILD_PTR(Term, 0)->token.type;
}