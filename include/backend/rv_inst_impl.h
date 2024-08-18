#ifndef RV_INST_IMPL_H
#define RV_INST_IMPL_H

#include "backend/rv_def.h"
#include <cstdint>
namespace rv {

struct rv_inst
{
    rvREG rd, rs1, rs2;      // operands of rv inst
    rvFREG frd, frs1, frs2;  // operands of rv inst
    rvOPCODE op;             // opcode of rv inst
    int imm;                 // optional, in immediate inst
    float fimm;
    std::string label;  // optional, in beq/jarl inst

    std::string draw() const;
    rv_inst(rvOPCODE op       = rvOPCODE::ADD,
            rvREG rd          = rvREG::X0,
            rvREG rs1         = rvREG::X0,
            rvREG rs2         = rvREG::X0,
            int imm           = 0,
            std::string label = "")  //
        : rd(rd), rs1(rs1), rs2(rs2), op(op), imm(imm), label(label)
    {
    }
    rv_inst(rvOPCODE op       = rvOPCODE::ADD,
            rvFREG rd         = rvFREG::F0,
            rvFREG rs1        = rvFREG::F0,
            rvFREG rs2        = rvFREG::F0,
            float imm         = 0,
            std::string label = "")  //
        : frd(rd), frs1(rs1), frs2(rs2), op(op), fimm(imm), label(label)
    {
    }
    rv_inst(rvOPCODE op       = rvOPCODE::ADD,
            rvFREG rd         = rvFREG::F0,
            rvREG rs1         = rvREG::X0,
            int imm           = 0,
            std::string label = "")  //
        : frd(rd), rs1(rs1), op(op), imm(imm), label(label)
    {
    }
    rv_inst(rvOPCODE op       = rvOPCODE::ADD,
            rvREG rd          = rvREG::X0,
            rvFREG rs1        = rvFREG::F0,
            int imm           = 0,
            std::string label = "")  //
        : rd(rd), frs1(rs1), op(op), imm(imm), label(label)
    {
    }
    rv_inst(rvOPCODE op       = rvOPCODE::ADD,
            rvREG rd          = rvREG::X0,
            rvFREG rs1        = rvFREG::F0,
            rvFREG rs2        = rvFREG::F0,
            float imm         = 0,
            std::string label = "")  //
        : rd(rd), frs1(rs1), frs2(rs2), op(op), fimm(imm), label(label)
    {
    }
};

};  // namespace rv

#endif