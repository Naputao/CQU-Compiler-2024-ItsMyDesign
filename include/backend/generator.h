#ifndef GENERARATOR_H
#define GENERARATOR_H

#include "backend/rv_def.h"
#include "backend/rv_inst_impl.h"
#include "ir/ir.h"

#include <cstdint>
#include <deque>
#include <fstream>
#include <list>
#include <string>
#include <unordered_map>
#include <vector>
namespace backend {

// it is a map bewteen variable and its mem addr, the mem addr of a local variable can be identified
// by ($sp + off)
struct stackVarMap
{
    std::unordered_map<std::string, int> _table;
    int offset = -56;
    /**
     * @brief find the addr of a ir::Operand
     * @return the offset
     */
    int find_operand(const std::string);

    /**
     * @brief add a ir::Operand into current map, alloc space for this variable in memory
     * @param[in] size: the space needed(in byte)
     * @return the offset
     */
    int add_operand(const std::string, uint32_t size = 4);
    stackVarMap(int offset = -56);
};
enum class VarArea { LOCAL,
                     PARAM,
                     GLOBL,
                     ANULL };
struct VarLocation
{
    bool isReg;
    VarArea vararea;
    int offset;
    rv::rvREG reg;
    rv::rvFREG freg;
    VarLocation(VarArea vararea = VarArea::LOCAL,
                int offset      = 0,
                bool isReg      = false,
                rv::rvREG reg   = rv::rvREG::X0,
                rv::rvFREG freg = rv::rvFREG::F0);
};

struct Generator
{
    const ir::Program& program;  // the program to gen
    std::ofstream& fout;         // output file
    std::list<std::string> sentences;
    std::list<std::string>::iterator globalSentenece;
    std::list<std::string>::iterator callSentenece;
    std::list<std::string>::iterator funcSentenece;
    stackVarMap* cur_varmap;
    stackVarMap* global_varmap;
    stackVarMap* param_varmap;
    std::vector<std::string>* regTag;
    // std::unordered_map<std::string, rv::rvREG>* regTable;
    std::deque<rv::rvREG> avaliableRegs;
    std::deque<rv::rvFREG> avaliableFRegs;
    Generator(ir::Program&, std::ofstream&);
    //
    VarLocation find_operand(const std::string name);
    void freereg(std::string);
    void freereg(const rv::rvREG);
    void freereg(const rv::rvFREG);
    // reg allocate api
    rv::rvREG getRd(const ir::Operand*);
    rv::rvREG getRs1(const ir::Operand*);
    rv::rvREG getRs2(const ir::Operand*);
    rv::rvFREG fgetRd(const ir::Operand*);
    rv::rvFREG fgetRs1(const ir::Operand*);
    rv::rvFREG fgetRs2(const ir::Operand*);
    rv::rvREG getarr(const ir::Operand*);
    rv::rvREG getnum(const std::string name);
    rv::rvFREG getfnum(const std::string name);
    // generate wrapper function
    void gen();
    void gen_func(const ir::Function&);
    void gen_instr(const ir::Instruction&);
    void gen_globalval(const std::vector<ir::GlobalVal>&);
    void gen_paramval(const std::vector<ir::Operand>&);
};
}  // namespace backend

#endif