#include "backend/rv_def.h"
std::string rv::toString(rvREG r)
{
    switch (r) {
        case rvREG::X0: return "zero";
        case rvREG::X1: return "ra";
        case rvREG::X2: return "sp";
        case rvREG::X3: return "gp";
        case rvREG::X4: return "tp";
        case rvREG::X5: return "t0";
        case rvREG::X6: return "t1";
        case rvREG::X7: return "t2";
        case rvREG::X8: return "s0";
        case rvREG::X9: return "s1";
        case rvREG::X10: return "a0";
        case rvREG::X11: return "a1";
        case rvREG::X12: return "a2";
        case rvREG::X13: return "a3";
        case rvREG::X14: return "a4";
        case rvREG::X15: return "a5";
        case rvREG::X16: return "a6";
        case rvREG::X17: return "a7";
        case rvREG::X18: return "s2";
        case rvREG::X19: return "s3";
        case rvREG::X20: return "s4";
        case rvREG::X21: return "s5";
        case rvREG::X22: return "s6";
        case rvREG::X23: return "s7";
        case rvREG::X24: return "s8";
        case rvREG::X25: return "s9";
        case rvREG::X26: return "s10";
        case rvREG::X27: return "s11";
        case rvREG::X28: return "t3";
        case rvREG::X29: return "t4";
        case rvREG::X30: return "t5";
        case rvREG::X31: return "t6";
        default: return "null";
    }
    return "null";
}  // implement this in ur own way
std::string rv::toString(rvFREG r)
{
    switch (r) {
        case rvFREG::F0: return "ft0";
        case rvFREG::F1: return "ft1";
        case rvFREG::F2: return "ft2";
        case rvFREG::F3: return "ft3";
        case rvFREG::F4: return "ft4";
        case rvFREG::F5: return "ft5";
        case rvFREG::F6: return "ft6";
        case rvFREG::F7: return "ft7";
        case rvFREG::F8: return "fs0";
        case rvFREG::F9: return "fs1";
        case rvFREG::F10: return "fa0";
        case rvFREG::F11: return "fa1";
        case rvFREG::F12: return "fa2";
        case rvFREG::F13: return "fa3";
        case rvFREG::F14: return "fa4";
        case rvFREG::F15: return "fa5";
        case rvFREG::F16: return "fa6";
        case rvFREG::F17: return "fa7";
        case rvFREG::F18: return "fs2";
        case rvFREG::F19: return "fs3";
        case rvFREG::F20: return "fs4";
        case rvFREG::F21: return "fs5";
        case rvFREG::F22: return "fs6";
        case rvFREG::F23: return "fs7";
        case rvFREG::F24: return "fs8";
        case rvFREG::F25: return "fs9";
        case rvFREG::F26: return "fs10";
        case rvFREG::F27: return "fs11";
        case rvFREG::F28: return "ft8";
        case rvFREG::F29: return "ft9";
        case rvFREG::F30: return "ft10";
        case rvFREG::F31: return "ft11";
        default: return "null123";
    }
    return "null";
}  // implement this in ur own way
std::string rv::toString(rvOPCODE r)
{
    switch (r) {
            // RV32I Base Integer Instructions
        case rvOPCODE::ADD: return "add";
        case rvOPCODE::SUB: return "sub";
        case rvOPCODE::XOR: return "xor";
        case rvOPCODE::OR: return "or";
        case rvOPCODE::AND: return "and";
        case rvOPCODE::SLL: return "sll";
        case rvOPCODE::SRL: return "slr";
        case rvOPCODE::SRA: return "sra";
        case rvOPCODE::SLT: return "slt";
        case rvOPCODE::SLTU: return "sltu";
        case rvOPCODE::ADDI: return "addi";
        case rvOPCODE::XORI: return "xori";
        case rvOPCODE::ORI: return "ori";
        case rvOPCODE::ANDI: return "andi";
        case rvOPCODE::SLLI: return "slli";
        case rvOPCODE::SRLI: return "srli";
        case rvOPCODE::SRAI: return "srai";
        case rvOPCODE::SLTI: return "slti";
        case rvOPCODE::SLTIU: return "sltiu";
        case rvOPCODE::LW: return "lw";
        case rvOPCODE::SW: return "sw";
        case rvOPCODE::BEQ: return "beq";
        case rvOPCODE::BNE: return "bne";
        case rvOPCODE::BLT: return "blt";
        case rvOPCODE::BGE: return "bge";
        case rvOPCODE::BLTU: return "bltu";
        case rvOPCODE::BGEU: return "bgeu";
        case rvOPCODE::JAL: return "jal";
        case rvOPCODE::JALR: return "jalr";
        case rvOPCODE::LA: return "la";
        case rvOPCODE::LI: return "li";
        case rvOPCODE::LUI: return "lui";
        case rvOPCODE::MOV: return "mv";
        case rvOPCODE::J: return "j";
        case rvOPCODE::JR: return "jr";
        case rvOPCODE::CALL: return "call";
        case rvOPCODE::MUL: return "mul";
        case rvOPCODE::DIV: return "div";
        case rvOPCODE::REM: return "rem";
        case rvOPCODE::SEQZ: return "seqz";
        case rvOPCODE::FLW: return "flw";
        case rvOPCODE::FSW: return "fsw";
        case rvOPCODE::FMADD_S: return "fmadd.s";
        case rvOPCODE::FMSUB_S: return "fmsub.s";
        case rvOPCODE::FNMSUB_S: return "fnmsub.s";
        case rvOPCODE::FNMADD_S: return "fnmadd.s";
        case rvOPCODE::FADD_S: return "fadd.s";
        case rvOPCODE::FSUB_S: return "fsub.s";
        case rvOPCODE::FMUL_S: return "fmul.s";
        case rvOPCODE::FDIV_S: return "fdiv.s";
        case rvOPCODE::FSQRT_S: return "fsqrt.s";
        case rvOPCODE::FSGNJ_S: return "fsgnj.s";
        case rvOPCODE::FSGNJN_S: return "fsgnjn.s";
        case rvOPCODE::FSGNJX_S: return "fsgnjx.s";
        case rvOPCODE::FMIN_S: return "fmin.s";
        case rvOPCODE::FMAX_S: return "fmax.s";
        case rvOPCODE::FCVT_W_S: return "fcvt.w.s";
        case rvOPCODE::FCVT_WU_S: return "fcvt.wu.s";
        case rvOPCODE::FMV_X_W: return "fmv.x.w";
        case rvOPCODE::FEQ_S: return "feq.s";
        case rvOPCODE::FLT_S: return "flt.s";
        case rvOPCODE::FLE_S: return "fle.s";
        case rvOPCODE::FCLASS_S: return "fclass.s";
        case rvOPCODE::FCVT_S_W: return "fcvt.s.w";
        case rvOPCODE::FCVT_S_WU: return "fcvt.s.wu";
        case rvOPCODE::FMV_W_X: return "fmv.w.x";
        default: return "unknown";
    }
    return "null";
}  // implement this in ur own way