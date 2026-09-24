#pragma once

#include <vector>
#include <string>

#include "token.hpp"

enum class opcode {
    not_opcode,
    NO_OPCODE,
    // アセンブラ命令
    START,
    END,
    DS,
    DC,
    // マクロ命令
    IN,
    OUT,
    RPUSH,
    RPOP,
    // ロード、ストア、ロードアドレス命令
    LD,
    LD_ADR,
    ST,
    LAD,
    // 算術、論理演算命令
    ADDA,
    ADDA_ADR,
    ADDL,
    ADDL_ADR,
    SUBA,
    SUBA_ADR,
    SUBL,
    SUBL_ADR,
    AND,
    AND_ADR,
    OR,
    OR_ADR,
    XOR,
    XOR_ADR,
    // 比較命令
    CPA,
    CPA_ADR,
    CPL,
    CPL_ADR,
    // シフト演算命令
    SLA,
    SRA,
    SLL,
    SRL,
    // 分岐命令
    JPL,
    JMI,
    JNZ,
    JZE,
    JOV,
    JUMP,
    PUSH,
    POP,
    // コール、リターン命令
    CALL,
    RET,
    // その他
    SVC,
    NOP,
    // 追加命令
    PUSHF,
    POPF,
    RETI,
    HLT,
    // アセンブラ用命令
    DATA, // メモリイメージ用命令

    num_opcodes,
};

enum class operands_type {
    none,
    register_,
    address,
    register_register,
    register_address,
    START,
    DS,
    DC,
    INOUT,
};

extern operands_type const operands_type_table[(size_t)opcode::num_opcodes];

struct operation {
    token label_;
    opcode opcode_;
    std::vector<token> operands_;

    size_t line_;

    operation();

    size_t get_code_length();
    bool is_comet2_operation();
};
