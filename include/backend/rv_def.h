#ifndef RV_DEF_H
#define RV_DEF_H

#include <string>

namespace rv {

// rv interger registers
enum class rvREG {
    /* Xn       its ABI name*/
    // .... FIX these comment by reading the risc-v ABI, and u should figure out the role of
    // every register in function call, including its saver(caller/callee)
    /*
    x0          Zero        constant —
    x1          ra          Return address Caller
    x2          sp          Stack pointer Callee
    x3          gp          Global pointer —
    x4          tp          Thread pointer —
    x5-x7       t0-t2       Temporaries Caller
    x8          s0 / fp     Saved / frame pointer Callee
    x9          s1          Saved register Callee
    x10-x11     a0-a1       Fn args/return values Caller
    x12-x17     a2-a7       Fn args Caller
    x18-x27     s2-s11      Saved registers Callee
    x28-x31     t3-t6       Temporaries Caller
    */
    // zero
    X0,   // zero
    X1,   // ra
    X2,   // sp
    X3,   // gp
    X4,   // tp
    X5,   // t0
    X6,   // t1
    X7,   // t2
    X8,   // s0/fp
    X9,   // s1
    X10,  // a0
    X11,  // a1
    X12,  // a2
    X13,  // a3
    X14,  // a4
    X15,  // a5
    X16,  // a6
    X17,  // a7
    X18,  // s2
    X19,  // s3
    X20,  // s4
    X21,  // s5
    X22,  // s6
    X23,  // s7
    X24,  // s8
    X25,  // s9
    X26,  // s10
    X27,  // s11
    X28,  // t3
    X29,  // t4
    X30,  // t5
    X31,  // t6
};
std::string toString(rvREG r);  // implement this in ur own way

enum class rvFREG {
    /*
    f0-7        ft0-7       FP temporaries Caller
    f8-9        fs0-1       FP saved registers Callee
    f10-11      fa0-1       FP args/return values Caller
    f12-17      fa2-7       FP args Caller
    f18-27      fs2-11      FP saved registers Callee
    f28-31      ft8-11      FP temporaries Caller
    */
    F0,   // ft0
    F1,   // ft1
    F2,   // ft2
    F3,   // ft3
    F4,   // ft4
    F5,   // ft5
    F6,   // ft6
    F7,   // ft7
    F8,   // fs0
    F9,   // fs1
    F10,  // fa0
    F11,  // fa1
    F12,  // fa2
    F13,  // fa3
    F14,  // fa4
    F15,  // fa5
    F16,  // fa6
    F17,  // fa7
    F18,  // fs2
    F19,  // fs3
    F20,  // fs4
    F21,  // fs5
    F22,  // fs6
    F23,  // fs7
    F24,  // fs8
    F25,  // fs9
    F26,  // fs10
    F27,  // fs11
    F28,  // ft8
    F29,  // ft9
    F30,  // ft10
    F31,  // ft11
};
std::string toString(rvFREG r);  // implement this in ur own way

// rv32i instructions
// add instruction u need here!
enum class rvOPCODE {
    // RV32I Base Integer Instructions
    ADD,
    SUB,
    XOR,
    OR,
    AND,
    SLL,
    SRL,
    SRA,
    SLT,
    SLTU,  // arithmetic & logic
    ADDI,
    XORI,
    ORI,
    ANDI,
    SLLI,
    SRLI,
    SRAI,
    SLTI,
    SLTIU,  // immediate
    LW,
    SW,  // load & store
    BEQ,
    BNE,
    BLT,
    BGE,
    BLTU,
    BGEU,  // conditional branch
    JAL,
    JALR,  // jump

    // RV32M Multiply Extension

    // RV32F / D Floating-Point Extensions

    // Pseudo Instructions
    LA,
    LI,
    LUI,
    MOV,
    J,  // ...
    JR,
    CALL,
    MUL,
    DIV,
    REM,
    SEQZ,
    FLW,
    FSW,
    FMADD_S,
    FMSUB_S,
    FNMSUB_S,
    FNMADD_S,
    FADD_S,
    FSUB_S,
    FMUL_S,
    FDIV_S,
    FSQRT_S,
    FSGNJ_S,
    FSGNJN_S,
    FSGNJX_S,
    FMIN_S,
    FMAX_S,
    FCVT_W_S,
    FCVT_WU_S,
    FMV_X_W,
    FEQ_S,
    FLT_S,
    FLE_S,
    FCLASS_S,
    FCVT_S_W,
    FCVT_S_WU,
    FMV_W_X,
};
std::string toString(rvOPCODE r);  // implement this in ur own way

}  // namespace rv

#endif
