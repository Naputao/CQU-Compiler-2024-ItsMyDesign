#include "backend/generator.h"
#include <assert.h>
#include <deque>
#include <unordered_map>
#define TODO assert(0 && "todo")
// #define REGLIST                                                                                 \
//     {                                                                                           \
//         rvREG::X5, rvREG::X6, rvREG::X7, rvREG::X28, rvREG::X29, rvREG::X30, rvREG::X31,        \
//             rvREG::X18, rvREG::X19, rvREG::X20, rvREG::X21, rvREG::X22, rvREG::X23, rvREG::X24, \
//             rvREG::X25, rvREG::X26, rvREG::X27, REGA0, rvREG::X11, rvREG::X12, rvREG::X13,      \
//             rvREG::X14, rvREG::X15, rvREG::X16, rvREG::X17, rvREG::X9                           \
//     }
#define REGLIST                           \
    {                                     \
        rvREG::X9, rvREG::X18, rvREG::X19 \
    }
#define NREG rvREG::X0
#define REGA0 rvREG::X10    // a0
#define REGA1 rvREG::X11    // a1
#define REGA2 rvREG::X12    // a2
#define REGA3 rvREG::X13    // a3
#define REGA4 rvREG::X14    // a4
#define REGA5 rvREG::X15    // a5
#define REGA6 rvREG::X16    // a6
#define REGA7 rvREG::X17    // a7
#define REGT0 rvREG::X5     // t0
#define REGT1 rvREG::X6     // t1
#define REGT2 rvREG::X7     // t2
#define REGT3 rvREG::X28    // t3
#define REGT4 rvREG::X29    // t4
#define REGT5 rvREG::X30    // t5
#define REGT6 rvREG::X31    // t6
#define REGRA rvREG::X1     // ra
#define REGSP rvREG::X2     // sp
#define REGS0 rvREG::X8     // s0
#define FREGA0 rvFREG::F10  // fa0
#define FREGT0 rvFREG::F5   // ft5
#define NIMM 0
#define FREGLIST                             \
    {                                        \
        rvFREG::F9, rvFREG::F18, rvFREG::F19 \
    }
#define NFREG rvFREG::F0

using backend::VarArea;
using backend::VarLocation;
using rv::rv_inst;
using rv::rvFREG;
using rv::rvOPCODE;
using rv::rvREG;
int frame_size      = 520;
int genInstrCounter = -1;
std::string cur_funcname;
backend::Generator::Generator(ir::Program& p, std::ofstream& f)
    : program(p), fout(f) {}
backend::VarLocation::VarLocation(VarArea vararea, int offset, bool isReg, rvREG reg, rvFREG)
    : vararea(vararea), offset(offset), reg(reg), freg(freg), isReg(isReg)
{
}
backend::stackVarMap::stackVarMap(int offset)
    : offset(offset) {}
bool isglobal(std::string str)
{
    std::string suffix = "_global";
    if (suffix.size() > str.size())
        return false;
    return std::equal(suffix.rbegin(), suffix.rend(), str.rbegin());
}
int backend::stackVarMap::find_operand(const std::string op)
{
    auto iter = this->_table.find(op);
    assert(iter != _table.end());
    return iter->second;
}
int backend::stackVarMap::add_operand(const std::string opname, uint32_t size)
{
    auto it = this->_table.find(opname);
    if (it != this->_table.end())
        return 0;
    this->_table[opname] = this->offset;
    this->offset         = this->offset - size;
    return 0;
}

VarLocation backend::Generator::find_operand(const std::string name)
{
    bool isReg  = false;
    rvREG reg   = NREG;
    rvFREG freg = NFREG;
    // auto regiter = this->regTable->find(name);
    // if (regiter != this->regTable->end()) {
    //     reg   = regiter->second;
    //     isReg = true;
    // }
    auto variter = this->cur_varmap->_table.find(name);
    if (variter != this->cur_varmap->_table.end())
        return VarLocation(VarArea::LOCAL, variter->second, isReg, reg, freg);
    variter = this->param_varmap->_table.find(name);
    if (variter != this->param_varmap->_table.end())
        return VarLocation(VarArea::PARAM, variter->second, isReg, reg, freg);
    variter = this->global_varmap->_table.find(name);
    if (variter != this->global_varmap->_table.end())
        return VarLocation(VarArea::GLOBL, variter->second, isReg, reg, freg);
    return VarLocation(VarArea::ANULL, -1, isReg, reg);
}
void backend::Generator::freereg(std::string name)
{
    VarLocation varloc = this->find_operand(name);
    rvREG base         = avaliableRegs.front();
    std::string cur_op_name;
    switch (varloc.vararea) {
        case VarArea::LOCAL:
        case VarArea::PARAM:
            this->sentences.push_back("\tsw\t" + toString(varloc.reg) + "," +
                                      std::to_string(varloc.offset) + "(s0)");
            break;
        case VarArea::GLOBL:
            this->avaliableRegs.pop_front();
            this->avaliableRegs.push_back(base);
            this->freereg(base);
            this->sentences.push_back(
                +"\t" +
                rv_inst(rvOPCODE::LA, base, NREG, NREG, NIMM, name).draw());
            this->sentences.push_back(
                +"\t" +
                rv_inst(rvOPCODE::SW, varloc.reg, base).draw());
            break;
        case VarArea::ANULL: break;
        default: assert(0);
    }
    this->regTag->at((int)varloc.reg) = "";
}

void backend::Generator::freereg(const rv::rvREG reg)
{
    std::string name = this->regTag->at((int)reg);
    if (name == "")
        return;
    VarLocation varloc = this->find_operand(name);
    rvREG base         = avaliableRegs.front();
    switch (varloc.vararea) {
        case VarArea::LOCAL:
        case VarArea::PARAM:
            this->sentences.push_back("\tsw\t" + toString(reg) + "," +
                                      std::to_string(varloc.offset) + "(s0)");
            break;
        case VarArea::GLOBL:
            this->avaliableRegs.pop_front();
            this->avaliableRegs.push_back(base);
            this->sentences.push_back(
                +"\t" +
                rv_inst(rvOPCODE::LA, base, NREG, NREG, NIMM, name).draw());
            this->sentences.push_back(
                +"\t" +
                rv_inst(rvOPCODE::SW, reg, base).draw());
            break;
        case VarArea::ANULL: break;
        default: assert(0);
    }
    this->regTag->at((int)reg) = "";
}
void backend::Generator::freereg(const rv::rvFREG freg)
{
    std::string name = this->regTag->at((int)freg + 32);
    if (name == "")
        return;
    VarLocation varloc = this->find_operand(name);
    rvREG base         = avaliableRegs.front();
    switch (varloc.vararea) {
        case VarArea::LOCAL:
        case VarArea::PARAM:
            this->sentences.push_back("\tfsw\t" + toString(freg) + "," +
                                      std::to_string(varloc.offset) + "(s0)");
            break;
        case VarArea::GLOBL:
            this->avaliableRegs.pop_front();
            this->avaliableRegs.push_back(base);
            this->sentences.push_back(
                +"\t" +
                rv_inst(rvOPCODE::LA, base, NREG, NREG, NIMM, name).draw());
            this->sentences.push_back(
                +"\t" +
                rv_inst(rvOPCODE::FSW, freg, base).draw());
            break;
        case VarArea::ANULL: break;
        default: assert(0);
    }
    this->regTag->at((int)freg + 32) = "";
}
rv::rvREG backend::Generator::getRd(const ir::Operand* op)
{
    VarLocation iter = this->find_operand(op->name);
    if (iter.isReg) {
        return iter.reg;
    }
    auto ret = avaliableRegs.front();
    avaliableRegs.pop_front();
    avaliableRegs.push_back(ret);
    // regTable->insert({op->name, ret});
    regTag->at((int)ret) = op->name;
    return ret;
}
rv::rvREG backend::Generator::getRs1(const ir::Operand* op)
{
    VarLocation iter = this->find_operand(op->name);
    rvREG ret        = avaliableRegs.front();
    avaliableRegs.pop_front();
    avaliableRegs.push_back(ret);
    // this->regTable->insert({op->name, ret});
    this->regTag->at((int)ret) = op->name;
    if (op->type == ir::Type::IntLiteral) {
        this->sentences.push_back(
            +"\t" + rv_inst(rvOPCODE::LI, ret, NREG, NREG, std::stoi(op->name)).draw());
        return ret;
    }
    switch (iter.vararea) {
        case VarArea::LOCAL:
            this->sentences.push_back("\tlw\t" + toString(ret) + "," + std::to_string(iter.offset) +
                                      "(s0)");
            break;
        case VarArea::PARAM:
            if (iter.offset >= 0)
                this->sentences.push_back("\tlw\t" + toString(ret) + "," +
                                          std::to_string(iter.offset) + "(s0)");
            else
                this->sentences.push_back("\tmv\t" + toString(ret) + "," +
                                          toString((rvREG)(18 + iter.offset / 4)));
            break;
        case VarArea::GLOBL:
            this->sentences.push_back(
                +"\t" +
                rv_inst(rvOPCODE::LA, ret, NREG, NREG, NIMM, op->name).draw());
            if (op->type != ir::Type::IntPtr)
                this->sentences.push_back(
                    +"\t" +
                    rv_inst(rvOPCODE::LW, ret, ret).draw());
            break;
        default: assert(0);
    }
    return ret;
}
rv::rvREG backend::Generator::getRs2(const ir::Operand* op)
{
    VarLocation iter = this->find_operand(op->name);
    rvREG ret        = avaliableRegs.front();
    avaliableRegs.pop_front();
    avaliableRegs.push_back(ret);
    // this->regTable->insert({op->name, ret});
    this->regTag->at((int)ret) = op->name;
    if (op->type == ir::Type::IntLiteral) {
        this->sentences.push_back(
            +"\t" + rv_inst(rvOPCODE::LI, ret, NREG, NREG, std::stoi(op->name)).draw());
        return ret;
    }
    switch (iter.vararea) {
        case VarArea::LOCAL:
            this->sentences.push_back("\tlw\t" + toString(ret) + "," + std::to_string(iter.offset) +
                                      "(s0)");
            break;
        case VarArea::PARAM:
            if (iter.offset >= 0)
                this->sentences.push_back("\tlw\t" + toString(ret) + "," +
                                          std::to_string(iter.offset) + "(s0)");
            else
                this->sentences.push_back("\tmv\t" + toString(ret) + "," +
                                          toString((rvREG)(18 + iter.offset / 4)));
            break;
        case VarArea::GLOBL:
            this->sentences.push_back(
                +"\t" +
                rv_inst(rvOPCODE::LA, ret, NREG, NREG, NIMM, op->name).draw());
            if (op->type != ir::Type::IntPtr)
                this->sentences.push_back(
                    +"\t" +
                    rv_inst(rvOPCODE::LW, ret, ret).draw());
            // this->sentences.push_back(
            //     +"\t" +
            //     rv_inst(rvOPCODE::LUI, ret, NREG, NREG, NIMM, "\%hi(" + op->name + ")").draw());
            // this->sentences.push_back(
            //     +"\t" +
            //     rv_inst(rvOPCODE::LW, ret, ret, NREG, NIMM, "\%lo(" + op->name + ")").draw());
            break;
        default: assert(0);
    }
    return ret;
}
rv::rvFREG backend::Generator::fgetRd(const ir::Operand* op)
{
    VarLocation iter = this->find_operand(op->name);
    if (iter.isReg) {
        return iter.freg;
    }
    auto ret = avaliableFRegs.front();
    avaliableFRegs.pop_front();
    avaliableFRegs.push_back(ret);
    // regTable->insert({op->name, ret});
    regTag->at((int)ret + 32) = op->name;
    return ret;
}
rv::rvFREG backend::Generator::fgetRs1(const ir::Operand* op)
{
    if (op->type == ir::Type::FloatLiteral) {
        return getfnum(op->name);
    }
    VarLocation iter = this->find_operand(op->name);
    rvFREG ret       = avaliableFRegs.front();
    avaliableFRegs.pop_front();
    avaliableFRegs.push_back(ret);
    // this->regTable->insert({op->name, ret});
    this->regTag->at((int)ret + 32) = op->name;

    switch (iter.vararea) {
        case VarArea::LOCAL:
            this->sentences.push_back("\tflw\t" + toString(ret) + "," + std::to_string(iter.offset) +
                                      "(s0)");
            break;
        case VarArea::PARAM:
            if (iter.offset >= 0)
                this->sentences.push_back("\tflw\t" + toString(ret) + "," +
                                          std::to_string(iter.offset) + "(s0)");
            else
                this->sentences.push_back("\t" + rv_inst(rv::rvOPCODE::FADD_S, ret, (rvFREG)(18 + iter.offset / 4), NFREG).draw());
            break;
        case VarArea::GLOBL:
            this->sentences.push_back(
                +"\t" +
                rv_inst(rvOPCODE::LA, ret, NFREG, NFREG, NIMM, op->name).draw());
            if (op->type != ir::Type::FloatPtr)
                this->sentences.push_back(
                    +"\t" +
                    rv_inst(rvOPCODE::FLW, ret, ret).draw());
            break;
        default: assert(0);
    }
    return ret;
}
rv::rvFREG backend::Generator::fgetRs2(const ir::Operand* op)
{
    if (op->type == ir::Type::FloatLiteral) {
        return getfnum(op->name);
    }
    VarLocation iter = this->find_operand(op->name);
    rvFREG ret       = avaliableFRegs.front();
    avaliableFRegs.pop_front();
    avaliableFRegs.push_back(ret);
    // this->regTable->insert({op->name, ret});
    this->regTag->at((int)ret + 32) = op->name;
    switch (iter.vararea) {
        case VarArea::LOCAL:
            this->sentences.push_back("\tflw\t" + toString(ret) + "," + std::to_string(iter.offset) +
                                      "(s0)");
            break;
        case VarArea::PARAM:
            if (iter.offset >= 0)
                this->sentences.push_back("\tflw\t" + toString(ret) + "," +
                                          std::to_string(iter.offset) + "(s0)");
            else
                this->sentences.push_back("\t" + rv_inst(rv::rvOPCODE::FADD_S, ret, (rvFREG)(18 + iter.offset / 4), NFREG).draw());
            break;
        case VarArea::GLOBL:
            this->sentences.push_back(
                +"\t" +
                rv_inst(rvOPCODE::LA, ret, NFREG, NFREG, NIMM, op->name).draw());
            if (op->type != ir::Type::FloatPtr)
                this->sentences.push_back(
                    +"\t" +
                    rv_inst(rvOPCODE::FLW, ret, ret).draw());
            break;
        default: assert(0);
    }
    return ret;
}
rv::rvREG backend::Generator::getarr(const ir::Operand* op)
{
    rvREG ret = avaliableRegs.front();
    avaliableRegs.pop_front();
    avaliableRegs.push_back(ret);
    // this->regTable->insert({op->name, ret});
    this->regTag->at((int)ret) = op->name;
    VarLocation iter           = this->find_operand(op->name);
    if (iter.vararea == VarArea::LOCAL) {
        return getRs1(op);
    }
    else {
        this->sentences.push_back(
            +"\t" + rv_inst(rvOPCODE::LA, ret, NREG, NREG, NIMM, op->name).draw());
        return ret;
    }
}
rv::rvREG backend::Generator::getnum(const std::string name)
{
    if (name == "0") {
        return NREG;
    }
    rvREG ret = avaliableRegs.front();
    avaliableRegs.pop_front();
    avaliableRegs.push_back(ret);
    // this->regTable->insert({name, ret});
    this->regTag->at((int)ret) = name;
    this->sentences.push_back(
        +"\t" + rv_inst(rvOPCODE::ADDI, ret, NREG, NREG, std::stoi(name)).draw());
    return ret;
}
int floatcnt = 0;
rv::rvFREG backend::Generator::getfnum(const std::string name)
{
    if (name == "0") {
        return NFREG;
    }
    rvFREG ret = avaliableFRegs.front();
    avaliableFRegs.pop_front();
    avaliableFRegs.push_back(ret);
    this->regTag->at((int)ret + 32) = name;
    this->sentences.insert(this->globalSentenece, "Floatnum" + std::to_string(floatcnt) + ":");
    this->sentences.insert(this->globalSentenece, "\t.float " + name);
    rvREG base = avaliableRegs.front();
    avaliableRegs.pop_front();
    avaliableRegs.push_back(base);
    this->sentences.push_back("\t" + rv_inst(rvOPCODE::LA, base, NREG, NREG, NIMM, "Floatnum" + std::to_string(floatcnt++)).draw());
    this->sentences.push_back("\t" + rv_inst(rvOPCODE::FLW, ret, base).draw());
    return ret;
}
void backend::Generator::gen()
{
    this->sentences.push_back("\t.option\tnopic");

    this->global_varmap = new stackVarMap();
    gen_globalval(program.globalVal);
    this->sentences.push_back("\t.text");
    this->globalSentenece = --this->sentences.end();
    for (auto func : program.functions)
        gen_func(func);
    int n = 0;
    for (const auto& sentence : this->sentences) {
        this->fout << sentence << "\n";
    }
    delete this->global_varmap;
}

void backend::Generator::gen_func(const ir::Function& func)
{
    // refresh
    this->cur_varmap   = new stackVarMap();
    this->param_varmap = new stackVarMap(0);
    this->regTag       = new std::vector<std::string>(64, "");
    cur_funcname       = func.name;
    // this->regTable      = new std::unordered_map<std::string, rv::rvREG>;
    this->avaliableRegs  = REGLIST;
    this->avaliableFRegs = FREGLIST;
    // basic info
    this->sentences.push_back("\t.align\t1");
    this->sentences.push_back("\t.globl\t" + func.name);
    this->sentences.push_back("\t.type\t" + func.name + ",\t" + "@function");
    this->sentences.push_back(func.name + ":");
    // save reg s0-s11 and set sp s0
    this->sentences.push_back("\taddi\tsp,sp,-" + std::to_string(frame_size));
    this->sentences.push_back("\tsw\tra," + std::to_string(frame_size - 4) + "(sp)");
    this->sentences.push_back("\tsw\ts0," + std::to_string(frame_size - 8) + "(sp)");
    this->sentences.push_back("\tsw\ts1," + std::to_string(frame_size - 12) + "(sp)");
    this->sentences.push_back("\tsw\ts2," + std::to_string(frame_size - 16) + "(sp)");
    this->sentences.push_back("\tsw\ts3," + std::to_string(frame_size - 20) + "(sp)");

    // this->sentences.push_back("\tsw\ts4," + std::to_string(frame_size - 24) + "(sp)");
    // this->sentences.push_back("\tsw\ts5," + std::to_string(frame_size - 28) + "(sp)");
    // this->sentences.push_back("\tsw\ts6," + std::to_string(frame_size - 32) + "(sp)");
    // this->sentences.push_back("\tsw\ts7," + std::to_string(frame_size - 36) + "(sp)");
    // this->sentences.push_back("\tsw\ts8," + std::to_string(frame_size - 40) + "(sp)");
    // this->sentences.push_back("\tsw\ts9," + std::to_string(frame_size - 44) + "(sp)");
    // this->sentences.push_back("\tsw\ts10," + std::to_string(frame_size - 48) + "(sp)");
    // this->sentences.push_back("\tsw\ts11," + std::to_string(frame_size - 52) + "(sp)");
    this->sentences.push_back("\taddi\ts0,sp," + std::to_string(frame_size));
    this->sentences.push_back("\tfmv.w.x\tft0,zero");

    gen_paramval(func.ParameterList);
    for (auto instr : func.InstVec) {
        // dbg
        this->sentences.push_back("#" + std::to_string(genInstrCounter + 1) + ": " + instr->draw());
        //
        gen_instr(*instr);
    }
    // load reg s0-s11 and reset sp s0
    this->sentences.push_back(cur_funcname + "_return:");
    this->sentences.push_back("\tlw\tra," + std::to_string(frame_size - 4) + "(sp)");
    this->sentences.push_back("\tlw\ts0," + std::to_string(frame_size - 8) + "(sp)");
    this->sentences.push_back("\tlw\ts1," + std::to_string(frame_size - 12) + "(sp)");
    this->sentences.push_back("\tlw\ts2," + std::to_string(frame_size - 16) + "(sp)");
    this->sentences.push_back("\tlw\ts3," + std::to_string(frame_size - 20) + "(sp)");
    // this->sentences.push_back("\tlw\ts4," + std::to_string(frame_size - 24) + "(sp)");
    // this->sentences.push_back("\tlw\ts5," + std::to_string(frame_size - 28) + "(sp)");
    // this->sentences.push_back("\tlw\ts6," + std::to_string(frame_size - 32) + "(sp)");
    // this->sentences.push_back("\tlw\ts7," + std::to_string(frame_size - 36) + "(sp)");
    // this->sentences.push_back("\tlw\ts8," + std::to_string(frame_size - 40) + "(sp)");
    // this->sentences.push_back("\tlw\ts9," + std::to_string(frame_size - 44) + "(sp)");
    // this->sentences.push_back("\tlw\ts10," + std::to_string(frame_size - 48) + "(sp)");
    // this->sentences.push_back("\tlw\ts11," + std::to_string(frame_size - 52) + "(sp)");
    this->sentences.push_back("\taddi\tsp,sp," + std::to_string(frame_size));
    this->sentences.push_back("\tjr\tra");
    this->sentences.push_back("\t.size\t" + func.name + ",\t.-" + func.name);
    delete this->cur_varmap;
    delete this->param_varmap;
    // delete this->regTable;
    delete this->regTag;
}
std::vector<std::list<std::string>::iterator> pc_ir2riscv;
std::unordered_multimap<int, std::string> GotoCounter;

void backend::Generator::gen_instr(const ir::Instruction& instr)
{
    pc_ir2riscv.push_back(--this->sentences.end());
    genInstrCounter++;
    auto itor = GotoCounter.equal_range(genInstrCounter);
    for (auto it = itor.first; it != itor.second; it = GotoCounter.erase(it)) {
        this->sentences.push_back(it->second + ":");
    }
    rvREG rd, rs1, rs2;
    rvFREG frd, frs1, frs2;
    int argument_cnt  = 0;
    int fargument_cnt = 0;
    VarLocation varlocation;
    int offset = 0;
    switch (instr.op) {
        case ir::Operator::add:
            rd  = getRd(&instr.des);
            rs1 = getRs1(&instr.op1);
            rs2 = getRs2(&instr.op2);
            this->sentences.push_back("\t" + rv_inst(rvOPCODE::ADD, rd, rs1, rs2).draw());
            this->freereg(rd);
            break;
        case ir::Operator::_goto:
            offset = std::stoi(instr.des.name);
            if (offset == 1)
                break;
            if (instr.op1.type != ir::Type::null) {
                if (instr.op1.type == ir::Type::FloatLiteral) {
                    if (std::stof(instr.op1.name)) {
                        if (offset > 0) {
                            this->sentences.push_back(
                                "\t" + rv_inst(rvOPCODE::J, NREG, NREG, NREG, NIMM, ".MY" + std::to_string(genInstrCounter)).draw());
                            GotoCounter.insert({genInstrCounter + offset, ".MY" + std::to_string(genInstrCounter)});
                        }
                        else {
                            this->sentences.insert(std::next(pc_ir2riscv[genInstrCounter + offset]), ".MY" + std::to_string(genInstrCounter) + ":");
                            this->sentences.push_back(
                                "\t" + rv_inst(rvOPCODE::J, NREG, NREG, NREG, NIMM, ".MY" + std::to_string(genInstrCounter)).draw());
                        }
                    }
                }
                else {
                    rs1 = getRs1(&instr.op1);
                    if (offset > 0) {
                        this->sentences.push_back(
                            "\t" + rv_inst(rvOPCODE::BNE, rs1, NREG, NREG, NIMM, ".MY" + std::to_string(genInstrCounter)).draw());
                        GotoCounter.insert({genInstrCounter + offset, ".MY" + std::to_string(genInstrCounter)});
                    }
                    else {
                        this->sentences.insert(std::next(pc_ir2riscv[genInstrCounter + offset]), ".MY" + std::to_string(genInstrCounter) + ":");
                        this->sentences.push_back(
                            "\t" + rv_inst(rvOPCODE::J, NREG, NREG, NREG, NIMM, ".MY" + std::to_string(genInstrCounter)).draw());
                    }
                }
            }
            else {
                if (offset > 0) {
                    this->sentences.push_back(
                        "\t" + rv_inst(rvOPCODE::J, NREG, NREG, NREG, NIMM, ".MY" + std::to_string(genInstrCounter)).draw());
                    GotoCounter.insert({genInstrCounter + offset, ".MY" + std::to_string(genInstrCounter)});
                }
                else {
                    this->sentences.insert(std::next(pc_ir2riscv[genInstrCounter + offset]), ".MY" + std::to_string(genInstrCounter) + ":");
                    this->sentences.push_back(
                        "\t" + rv_inst(rvOPCODE::J, NREG, NREG, NREG, NIMM, ".MY" + std::to_string(genInstrCounter)).draw());
                }
            }
            break;
        case ir::Operator::call:
            for (auto argument : dynamic_cast<const ir::CallInst&>(instr).argumentList) {
                if (argument.type == ir::Type::FloatLiteral || argument.type == ir::Type::Float) {
                    if (fargument_cnt < 8) {
                        frd  = (rvFREG)(10 + fargument_cnt);
                        frs1 = fgetRs1(&argument);
                        this->sentences.push_back("\t" + rv_inst(rvOPCODE::FADD_S, frd, frs1).draw());
                    }
                    else {
                        assert(0 && "Not Realized");
                    }
                    fargument_cnt++;
                }
                else {
                    if (argument_cnt < 8) {
                        rd  = (rvREG)(10 + argument_cnt);
                        rs1 = getRs1(&argument);
                        this->sentences.push_back("\t" + rv_inst(rvOPCODE::MOV, rd, rs1).draw());
                    }
                    else {
                        rs1 = getRs1(&argument);
                        this->sentences.push_back(
                            +"\t" +
                            rv_inst(rvOPCODE::SW, rs1, rvREG::X2, NREG, 4 * argument_cnt - 32).draw());
                    }
                }

                argument_cnt++;
            }
            this->sentences.push_back(
                +"\t" + rv_inst(rvOPCODE::CALL, NREG, NREG, NREG, NIMM, instr.op1.name).draw());

            if (instr.des.type == ir::Type::null) {
                break;
            }
            if (instr.des.type == ir::Type::Int) {
                rd = getRd(&instr.des);
                this->sentences.push_back(
                    +"\t" + rv_inst(rvOPCODE::MOV, rd, REGA0, NREG, NIMM, instr.op1.name).draw());
                this->freereg(rd);
            }
            else {
                frd = fgetRd(&instr.des);
                this->sentences.push_back(
                    +"\t" + rv_inst(rvOPCODE::FADD_S, frd, FREGA0, NFREG, NIMM, instr.op1.name).draw());
                this->freereg(frd);
            }
            break;
        case ir::Operator::alloc:
            varlocation = find_operand(instr.des.name);
            if (varlocation.vararea == VarArea::GLOBL)
                break;
            this->cur_varmap->add_operand(instr.des.name);
            this->sentences.insert(this->globalSentenece, "\t.data");
            this->sentences.insert(this->globalSentenece, instr.des.name + ":");
            this->sentences.insert(this->globalSentenece, "\t.space\t" + std::to_string(std::stoi(instr.op1.name) * 4));
            this->global_varmap->add_operand(instr.des.name);
            rd = this->getRd(&instr.des);
            this->sentences.push_back("\t" + rv_inst(rvOPCODE::LA, rd, NREG, NREG, NIMM, instr.des.name).draw());
            this->freereg(rd);
            break;
        case ir::Operator::store:
            rs1 = getarr(&instr.op1);
            rs2 = getRs1(&instr.op2);
            this->sentences.push_back(
                "\t" + rv_inst(rvOPCODE::SLLI, rs2, rs2, NREG, 2).draw());
            this->sentences.push_back(
                "\t" + rv_inst(rvOPCODE::ADD, rs1, rs1, rs2).draw());
            if (instr.op1.type == ir::Type::IntPtr) {
                rd = getRs2(&instr.des);
                this->sentences.push_back(
                    "\t" + rv_inst(rvOPCODE::SW, rd, rs1).draw());
            }
            else {
                frd = fgetRs2(&instr.des);
                this->sentences.push_back(
                    "\t" + rv_inst(rvOPCODE::FSW, frd, rs1).draw());
            }

            break;
        case ir::Operator::load:
            varlocation = find_operand(instr.des.name);
            if (varlocation.vararea != VarArea::GLOBL)
                this->cur_varmap->add_operand(instr.des.name);
            rs2 = getRs2(&instr.op2);
            rs1 = getarr(&instr.op1);
            rd  = getRd(&instr.des);
            this->sentences.push_back("\t" + rv_inst(rvOPCODE::SLLI, rs2, rs2, NREG, 2).draw());
            this->sentences.push_back("\t" + rv_inst(rvOPCODE::ADD, rd, rs1, rs2).draw());
            this->sentences.push_back("\t" + rv_inst(rvOPCODE::LW, rd, rd).draw());
            this->freereg(rd);
            break;
        case ir::Operator::getptr: break;
        case ir::Operator::def:
            varlocation = find_operand(instr.des.name);
            if (varlocation.vararea != VarArea::GLOBL)
                this->cur_varmap->add_operand(instr.des.name);
            rd  = getRd(&instr.des);
            rs1 = instr.op1.name == "0" ? NREG : getRs1(&instr.op1);
            this->sentences.push_back("\t" + rv_inst(rvOPCODE::MOV, rd, rs1).draw());
            this->freereg(rd);
            break;
        case ir::Operator::fdef:
            varlocation = find_operand(instr.des.name);
            if (varlocation.vararea != VarArea::GLOBL)
                this->cur_varmap->add_operand(instr.des.name);
            frd  = fgetRd(&instr.des);
            frs1 = instr.op1.name == "0" ? NFREG : fgetRs1(&instr.op1);
            this->sentences.push_back("\t" + rv_inst(rvOPCODE::FADD_S, frd, frs1, NFREG).draw());
            this->freereg(frd);
            break;
        case ir::Operator::mov:
            rd  = getRd(&instr.des);
            rs1 = getRs1(&instr.op1);
            this->sentences.push_back("\t" + rv_inst(rvOPCODE::MOV, rd, rs1, NREG, NIMM).draw());
            this->freereg(rd);
            break;
        case ir::Operator::fmov:
            frd  = fgetRd(&instr.des);
            frs1 = fgetRs1(&instr.op1);
            this->sentences.push_back("\t" + rv_inst(rvOPCODE::FADD_S, frd, frs1, NFREG).draw());
            this->freereg(frd);

            break;
        case ir::Operator::cvt_i2f:
            this->cur_varmap->add_operand(instr.des.name);
            frd = fgetRd(&instr.des);
            rs1 = getRs1(&instr.op1);
            this->sentences.push_back("\tfrcsr\tt0");
            this->sentences.push_back("\tfsrmi\t2");
            this->sentences.push_back("\t" + rv_inst(rvOPCODE::FCVT_S_W, frd, rs1).draw());
            this->sentences.push_back("\tfscsr\tt0");
            this->freereg(frd);
            break;

        case ir::Operator::cvt_f2i:
            this->cur_varmap->add_operand(instr.des.name);
            rd   = getRd(&instr.des);
            frs1 = fgetRs1(&instr.op1);
            this->sentences.push_back("\tfrcsr\tt0");
            this->sentences.push_back("\tfsrmi\t2");
            this->sentences.push_back("\t" + rv_inst(rvOPCODE::FCVT_W_S, rd, frs1, NIMM).draw());
            this->sentences.push_back("\tfscsr\tt0");
            this->freereg(rd);
            break;
        case ir::Operator::addi:
            rd  = getRd(&instr.des);
            rs1 = getRs1(&instr.op1);
            this->sentences.push_back(
                +"\t" + rv_inst(rvOPCODE::ADDI, rd, rs1, NREG, std::stoi(instr.op2.name)).draw());
            this->freereg(rd);
            break;
        case ir::Operator::fadd:
            frd  = fgetRd(&instr.des);
            frs1 = fgetRs1(&instr.op1);
            frs2 = fgetRs2(&instr.op2);
            this->sentences.push_back("\t" + rv_inst(rvOPCODE::FADD_S, frd, frs1, frs2).draw());
            this->freereg(frd);
            break;
        case ir::Operator::sub:
            this->cur_varmap->add_operand(instr.des.name);
            rd  = getRd(&instr.des);
            rs1 = getRs1(&instr.op1);
            rs2 = getRs2(&instr.op2);
            this->sentences.push_back("\t" + rv_inst(rvOPCODE::SUB, rd, rs1, rs2).draw());
            this->freereg(rd);
            break;
        case ir::Operator::subi:
            rd  = getRd(&instr.des);
            rs1 = getRs1(&instr.op1);
            this->sentences.push_back(
                +"\t" + rv_inst(rvOPCODE::ADDI, rd, rs1, NREG, -std::stoi(instr.op2.name)).draw());
            this->freereg(rd);
            break;
        case ir::Operator::fsub:
            this->cur_varmap->add_operand(instr.des.name);
            frd  = fgetRd(&instr.des);
            frs1 = fgetRs1(&instr.op1);
            frs2 = fgetRs2(&instr.op2);
            this->sentences.push_back("\t" + rv_inst(rvOPCODE::FSUB_S, frd, frs1, frs2).draw());
            this->freereg(frd);
            break;
        case ir::Operator::mul:
            rd  = getRd(&instr.des);
            rs1 = getRs1(&instr.op1);
            rs2 = getRs2(&instr.op2);
            this->sentences.push_back("\t" + rv_inst(rvOPCODE::MUL, rd, rs1, rs2).draw());
            this->freereg(rd);
            break;
        case ir::Operator::fmul:
            frd  = fgetRd(&instr.des);
            frs1 = fgetRs1(&instr.op1);
            frs2 = fgetRs2(&instr.op2);
            this->sentences.push_back("\t" + rv_inst(rvOPCODE::FMUL_S, frd, frs1, frs2).draw());
            this->freereg(frd);
            break;
        case ir::Operator::div:
            rd  = getRd(&instr.des);
            rs1 = getRs1(&instr.op1);
            rs2 = getRs2(&instr.op2);
            this->sentences.push_back("\t" + rv_inst(rvOPCODE::DIV, rd, rs1, rs2).draw());
            this->freereg(rd);
            break;
        case ir::Operator::fdiv:
            frd  = fgetRd(&instr.des);
            frs1 = fgetRs1(&instr.op1);
            frs2 = fgetRs2(&instr.op2);
            this->sentences.push_back("\t" + rv_inst(rvOPCODE::FDIV_S, frd, frs1, frs2).draw());
            this->freereg(frd);
            break;
        case ir::Operator::mod:
            rd  = getRd(&instr.des);
            rs1 = getRs1(&instr.op1);
            rs2 = getRs2(&instr.op2);
            this->sentences.push_back("\t" + rv_inst(rvOPCODE::REM, rd, rs1, rs2).draw());
            this->freereg(rd);
            break;
        case ir::Operator::lss:
            rd  = getRd(&instr.des);
            rs1 = getRs1(&instr.op1);
            rs2 = getRs2(&instr.op2);
            this->sentences.push_back("\t" + rv_inst(rvOPCODE::SLT, rd, rs1, rs2).draw());
            this->freereg(rd);
            break;
        case ir::Operator::flss:
            rd   = getRd(&instr.des);
            frs1 = fgetRs1(&instr.op1);
            frs2 = fgetRs2(&instr.op2);
            this->sentences.push_back("\t" + rv_inst(rvOPCODE::FLT_S, rd, frs1, frs2).draw());
            this->freereg(rd);
            break;
        case ir::Operator::leq:
            rd  = getRd(&instr.des);
            rs1 = getRs1(&instr.op1);
            rs2 = getRs2(&instr.op2);
            this->sentences.push_back("\t" + rv_inst(rvOPCODE::SLT, rd, rs2, rs1).draw());
            this->sentences.push_back("\t" + rv_inst(rvOPCODE::XORI, rd, rd, NREG, 1).draw());
            this->freereg(rd);
            break;
        case ir::Operator::fleq: break;
        case ir::Operator::gtr:
            rd  = getRd(&instr.des);
            rs1 = getRs1(&instr.op1);
            rs2 = getRs2(&instr.op2);
            this->sentences.push_back("\t" + rv_inst(rvOPCODE::SLT, rd, rs2, rs1).draw());
            this->freereg(rd);
            break;
        case ir::Operator::fgtr: break;
        case ir::Operator::geq:
            rd  = getRd(&instr.des);
            rs1 = getRs1(&instr.op1);
            rs2 = getRs2(&instr.op2);
            this->sentences.push_back("\t" + rv_inst(rvOPCODE::SLT, rd, rs1, rs2).draw());
            this->sentences.push_back("\t" + rv_inst(rvOPCODE::XORI, rd, rd, NREG, 1).draw());
            this->freereg(rd);
            break;
        case ir::Operator::fgeq: break;
        case ir::Operator::eq:
            rd  = getRd(&instr.des);
            rs1 = getRs1(&instr.op1);
            rs2 = getRs2(&instr.op2);
            this->sentences.push_back("\t" + rv_inst(rvOPCODE::XOR, rd, rs1, rs2).draw());
            this->sentences.push_back("\t" + rv_inst(rvOPCODE::SLTIU, rd, rd, NREG, 1).draw());
            this->freereg(rd);
            break;
        case ir::Operator::feq: break;
        case ir::Operator::neq:
            rd  = getRd(&instr.des);
            rs1 = getRs1(&instr.op1);
            rs2 = getRs2(&instr.op2);
            this->sentences.push_back("\t" + rv_inst(rvOPCODE::XOR, rd, rs1, rs2).draw());
            this->sentences.push_back("\t" + rv_inst(rvOPCODE::SLTIU, rd, rd, NREG, 1).draw());
            this->sentences.push_back("\t" + rv_inst(rvOPCODE::XORI, rd, rd, NREG, 1).draw());
            this->freereg(rd);
            break;
        case ir::Operator::fneq: break;
        case ir::Operator::_not:
            this->cur_varmap->add_operand(instr.des.name);
            rd  = getRd(&instr.des);
            rs1 = getRs1(&instr.op1);
            this->sentences.push_back("\t" + rv_inst(rvOPCODE::SEQZ, rd, rs1, rs2).draw());
            this->freereg(rd);
            break;
        case ir::Operator::_and:
            rs1 = getRs1(&instr.op1);
            rs2 = getRs2(&instr.op2);
            rd  = getRd(&instr.des);
            this->sentences.push_back("\t" + rv_inst(rvOPCODE::SLTIU, rs1, rs1, NREG, 1).draw());
            this->sentences.push_back("\t" + rv_inst(rvOPCODE::XORI, rs1, rs1, NREG, 1).draw());
            this->sentences.push_back("\t" + rv_inst(rvOPCODE::SLTIU, rs2, rs2, NREG, 1).draw());
            this->sentences.push_back("\t" + rv_inst(rvOPCODE::XORI, rs2, rs2, NREG, 1).draw());
            this->sentences.push_back("\t" + rv_inst(rvOPCODE::AND, rd, rs1, rs2).draw());

            this->freereg(rd);
            break;
        case ir::Operator::_or:
            rd  = getRd(&instr.des);
            rs1 = getRs1(&instr.op1);
            rs2 = getRs2(&instr.op2);
            this->sentences.push_back("\t" + rv_inst(rvOPCODE::OR, rd, rs1, rs2).draw());
            this->freereg(rd);
            break;
        case ir::Operator::_return:
            if (instr.op1.type == ir::Type::null)
                break;
            if (instr.op1.type == ir::Type::Int || instr.op1.type == ir::Type::IntLiteral) {
                rs1 = getRs1(&instr.op1);
                this->sentences.push_back("\t" + rv_inst(rvOPCODE::MOV, REGA0, rs1).draw());
            }
            else {
                frs1 = fgetRs1(&instr.op1);
                this->sentences.push_back("\t" + rv_inst(rvOPCODE::FADD_S, FREGA0, frs1).draw());
            }
            this->sentences.push_back("\t" + rv_inst(rvOPCODE::J, NREG, NREG, NREG, NIMM, cur_funcname + "_return").draw());
            break;
        case ir::Operator::__unuse__: break;
        default: assert(0 && "Unexpected Operater");
    }
}
void backend::Generator::gen_paramval(const std::vector<ir::Operand>& ParameterList)
{
    this->param_varmap->offset = -32;
    for (auto val : ParameterList) {
        if (val.type == ir::Type::Float || val.type == ir::Type::FloatLiteral) {
            if (this->param_varmap->offset >= 0)
                this->sentences.push_back("\t" + rv_inst(rvOPCODE::FLW, FREGT0, REGS0, this->param_varmap->offset).draw());
            else
                this->sentences.push_back("\t" + rv_inst(rvOPCODE::FADD_S, FREGT0, (rvFREG)(18 + this->param_varmap->offset / 4)).draw());
            this->sentences.push_back("\t" + rv_inst(rvOPCODE::FSW, FREGT0, REGS0, this->cur_varmap->offset).draw());
        }
        else {
            if (this->param_varmap->offset >= 0)
                this->sentences.push_back("\tlw\t" + toString(REGT0) + "," +
                                          std::to_string(this->param_varmap->offset) + "(s0)");
            else
                this->sentences.push_back("\tmv\t" + toString(REGT0) + "," +
                                          toString((rvREG)(18 + this->param_varmap->offset / 4)));
            this->sentences.push_back("\tsw\t" + toString(REGT0) + "," +
                                      std::to_string(this->cur_varmap->offset) + "(s0)");
        }
        this->param_varmap->add_operand(val.name, -4);
        this->cur_varmap->add_operand(val.name);
    }
}
void backend::Generator::gen_globalval(const std::vector<ir::GlobalVal>& GlobalvalList)
{
    this->sentences.push_back("\t.data");
    for (auto val : GlobalvalList) {
        if (val.val.type != ir::Type::IntLiteral && val.val.type != ir::Type::FloatLiteral) {
            if (val.maxlen == 0) {
                this->sentences.push_back(val.val.name + ":");
                this->sentences.push_back("\t.word\t0");
            }
            else {
                this->sentences.push_back(val.val.name + ":");
                this->sentences.push_back("\t.space\t" + std::to_string(val.maxlen * 4));
            }
            this->global_varmap->add_operand(val.val.name);
        }
    }
}
// this->sentences.push_back( + "\t.globl\t" + val.val.name );
// this->sentences.push_back( + "\t.section\t.sdata,\"aw\"" );
// this->sentences.push_back( + "\t.align\t2" );
// this->sentences.push_back( + "\t.type\t" + val.val.name + ",\t@object" );
// this->sentences.push_back( + "\t.size\t" + val.val.name + ",\t4" );
// this->sentences.push_back( + val.val.name + ":" );
// this->sentences.push_back( + "\t.word\t" + instr->op1.name );

// this->sentences.push_back( + "\t.globl\t" + val.val.name );
// this->sentences.push_back( + "\t.align\t2" );
// this->sentences.push_back( + "\t.type\t" + val.val.name + ",\t@object" );
// this->sentences.push_back( + "\t.size\t" + val.val.name + ",\t"
//            + std::to_string(std::stoi(instr->op1.name) * 4) );
// this->sentences.push_back( + val.val.name + ":" );

// rvREG backend::Generator::getRd(const ir::Operand* op)
// {
//     VarLocation iter = this->find_operand(op->name);
//     if (iter.isReg)
//         return iter.reg;
//     auto ret = avaliableRegs.front();
//     avaliableRegs.pop_front();
//     avaliableRegs.push_back(ret);
//     this->freereg(ret);
//     regTable->insert({op->name, ret});
//     regTag->at((int)ret) = op->name;
//     return ret;
// }

// rvFREG backend::Generator::fgetRd(const ir::Operand* op)
// {
//     TODO;
// }
// rvREG backend::Generator::getRs1(const ir::Operand* op)
// {
//     VarLocation iter = this->find_operand(op->name);
//     if (iter.isReg) {
//         return iter.reg;
//     }
//     rvREG ret = avaliableRegs.front();
//     avaliableRegs.pop_front();
//     avaliableRegs.push_back(ret);
//     this->freereg(ret);
//     this->regTable->insert({op->name, ret});
//     this->regTag->at((int)ret) = op->name;
//     if (op->type == ir::Type::IntLiteral) {
//         this->sentences.push_back(
//             +"\t" + rv_inst(rvOPCODE::ADDI, ret, NREG, NREG, std::stoi(op->name)).draw());
//         return ret;
//     }
//     switch (iter.vararea) {
//         case VarArea::LOCAL:
//             this->sentences.push_back("\tlw\t" + toString(ret) + "," + std::to_string(iter.offset) +
//                                       "(s0)");
//             break;
//         case VarArea::PARAM:
//             if (iter.offset >= 0)
//                 this->sentences.push_back("\tlw\t" + toString(ret) + "," +
//                                           std::to_string(iter.offset) + "(s0)");
//             else
//                 this->sentences.push_back("\tmv\t" + toString(ret) + "," +
//                                           toString((rvREG)(18 + iter.offset / 4)));
//             break;
//         case VarArea::GLOBL:
//             this->sentences.push_back(
//                 +"\t" +
//                 rv_inst(rvOPCODE::LUI, ret, NREG, NREG, NIMM, "\%hi(" + op->name + ")").draw());
//             this->sentences.push_back(
//                 +"\t" +
//                 rv_inst(rvOPCODE::LW, ret, ret, NREG, NIMM, "\%lo(" + op->name + ")").draw());
//             break;
//         default: assert(0);
//     }
//     return ret;
// }
// rvREG backend::Generator::getRs2(const ir::Operand* op)
// {
//     VarLocation iter = this->find_operand(op->name);
//     if (iter.isReg) {
//         return iter.reg;
//     }
//     rvREG ret = avaliableRegs.front();
//     avaliableRegs.pop_front();
//     avaliableRegs.push_back(ret);
//     this->freereg(ret);
//     this->regTable->insert({op->name, ret});
//     this->regTag->at((int)ret) = op->name;
//     if (op->type == ir::Type::IntLiteral) {
//         this->sentences.push_back(
//             +"\t" + rv_inst(rvOPCODE::ADDI, ret, NREG, NREG, std::stoi(op->name)).draw());
//         return ret;
//     }
//     switch (iter.vararea) {
//         case VarArea::LOCAL:
//             this->sentences.push_back("\tlw\t" + toString(ret) + "," + std::to_string(iter.offset) +
//                                       "(s0)");
//             break;
//         case VarArea::PARAM:
//             if (iter.offset >= 0)
//                 this->sentences.push_back("\tlw\t" + toString(ret) + "," +
//                                           std::to_string(iter.offset) + "(s0)");
//             else
//                 this->sentences.push_back("\tmv\t" + toString(ret) + "," +
//                                           toString((rvREG)(18 + iter.offset / 4)));
//             break;
//         case VarArea::GLOBL:
//             this->sentences.push_back(
//                 +"\t" +
//                 rv_inst(rvOPCODE::LUI, ret, NREG, NREG, NIMM, "\%hi(" + op->name + ")").draw());
//             this->sentences.push_back(
//                 +"\t" +
//                 rv_inst(rvOPCODE::LW, ret, ret, NREG, NIMM, "\%lo(" + op->name + ")").draw());
//             break;
//         default: assert(0);
//     }
//     return ret;
// }
// rvREG backend::Generator::getarr(const ir::Operand* op)
// {
//     auto iter = this->regTable->find(op->name);
//     if (iter != this->regTable->end()) {
//         return iter->second;
//     }
//     rvREG ret = avaliableRegs.front();
//     avaliableRegs.pop_front();
//     avaliableRegs.push_back(ret);
//     this->freereg(ret);
//     this->regTable->insert({op->name, ret});
//     this->regTag->at((int)ret) = op->name;
//     this->sentences.push_back(
//         +"\t" + rv_inst(rvOPCODE::LUI, ret, NREG, NREG, NIMM, "\%hi(" + op->name + ")").draw());
//     this->sentences.push_back(
//         +"\t" + rv_inst(rvOPCODE::ADDI, ret, ret, NREG, NIMM, "\%lo(" + op->name + ")").draw());
//     return ret;
// }
// rv::rvREG backend::Generator::getnum(const std::string name)
// {
//     auto iter = this->regTable->find(name);
//     if (iter != this->regTable->end()) {
//         return iter->second;
//     }
//     rvREG ret = avaliableRegs.front();
//     avaliableRegs.pop_front();
//     avaliableRegs.push_back(ret);
//     this->freereg(ret);
//     this->regTable->insert({name, ret});
//     this->regTag->at((int)ret) = name;
//     this->sentences.push_back(
//         +"\t" + rv_inst(rvOPCODE::ADDI, ret, NREG, NREG, std::stoi(name)).draw());
//     return ret;
// }
// rvFREG backend::Generator::fgetRs1(const ir::Operand* op)
// {
//     TODO;
// }
// rvFREG backend::Generator::fgetRs2(const ir::Operand* op)
// {
//     TODO;
// }