#include "backend/rv_inst_impl.h"
#include "backend/rv_def.h"
#define OP toString(this->op)
#define RD toString(this->rd)
#define RS1 toString(this->rs1)
#define RS2 toString(this->rs2)
#define IMM std::to_string(this->imm)
#define LABEL this->label
#define FRD toString(this->frd)
#define FRS1 toString(this->frs1)
#define FRS2 toString(this->frs2)
#define FIMM std::to_string(this->fimm)
#define OFFSET (LABEL.empty() ? IMM : (LABEL == "NULL" ? "" : LABEL))
#define FOFFSET (LABEL.empty() ? FIMM : (LABEL == "NULL" ? "" : LABEL))
namespace rv {
std::string rv_inst::draw() const
{
    switch (op) {
            // RV32I Base Integer Instructions
        case rvOPCODE::ADD:
        case rvOPCODE::SUB:
        case rvOPCODE::XOR:
        case rvOPCODE::OR:
        case rvOPCODE::AND:
        case rvOPCODE::SLL:
        case rvOPCODE::SRL:
        case rvOPCODE::SRA:
        case rvOPCODE::SLT:
        case rvOPCODE::SLTU:  // arithmetic & logic
            return OP + " " + RD + ", " + RS1 + ", " + RS2;
        case rvOPCODE::ADDI:
        case rvOPCODE::XORI:
        case rvOPCODE::ORI:
        case rvOPCODE::ANDI:
        case rvOPCODE::SLLI:
        case rvOPCODE::SRLI:
        case rvOPCODE::SRAI:
        case rvOPCODE::SLTI:
        case rvOPCODE::SLTIU:  // immediate
            return OP + "\t" + RD + "," + RS1 + "," + OFFSET;
        case rvOPCODE::LW:
        case rvOPCODE::SW:  // load & store
            return OP + "\t" + RD + "," + OFFSET + "(" + RS1 + ")";
        case rvOPCODE::BEQ:
        case rvOPCODE::BNE:
            return OP + "\t" + RD + "," + RS1 + "," + OFFSET;
        case rvOPCODE::BLT: return "null";
        case rvOPCODE::BGE: return "null";
        case rvOPCODE::BLTU: return "null";
        case rvOPCODE::BGEU:  // conditional branch
            return OP + "\t" + RS1 + "," + RS2 + "," + LABEL;
        case rvOPCODE::JAL: return "null";
        case rvOPCODE::JALR:  // jump
            return "null";
            // RV32M Multiply Extension
            // RV32F / D Floating-Point Extensions// Pseudo Instructions
        case rvOPCODE::LA: return OP + "\t" + RD + "," + LABEL;
        case rvOPCODE::LI: return OP + "\t" + RD + "," + OFFSET;
        case rvOPCODE::LUI: return OP + "\t" + RD + "," + OFFSET;
        case rvOPCODE::MOV: return OP + "\t" + RD + "," + RS1;
        case rvOPCODE::J:  // ...
            return OP + "\t" + LABEL;
        case rvOPCODE::JR: return OP + "\t" + RD;
        case rvOPCODE::CALL: return OP + "\t" + LABEL;
        case rvOPCODE::MUL: return OP + " " + RD + ", " + RS1 + ", " + RS2;
        case rvOPCODE::DIV: return OP + " " + RD + ", " + RS1 + ", " + RS2;
        case rvOPCODE::REM: return OP + " " + RD + ", " + RS1 + ", " + RS2;
        case rvOPCODE::SEQZ: return OP + " " + RD + ", " + RS1;
        case rvOPCODE::FLW: return OP + "\t" + FRD + "," + OFFSET + "(" + RS1 + ")";
        case rvOPCODE::FSW: return OP + "\t" + FRD + "," + OFFSET + "(" + RS1 + ")";
        case rvOPCODE::FCVT_W_S: return OP + "\t" + RD + "," + FRS1;
        case rvOPCODE::FCVT_S_W: return OP + "\t" + FRD + "," + RS1;
        case rvOPCODE::FADD_S: return OP + "\t" + FRD + "," + FRS1 + "," + FRS2;
        case rvOPCODE::FSUB_S: return OP + "\t" + FRD + "," + FRS1 + "," + FRS2;
        case rvOPCODE::FMUL_S: return OP + "\t" + FRD + "," + FRS1 + "," + FRS2;
        case rvOPCODE::FDIV_S: return OP + "\t" + FRD + "," + FRS1 + "," + FRS2;
        case rvOPCODE::FLT_S: return OP + "\t" + RD + "," + FRS1 + "," + FRS2;
        default: break;
    }
    return "null";
}
}  // namespace rv
