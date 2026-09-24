#include <stdexcept>

#include "operation.hpp"

// opcodeとオペランドの書き方の対応表
operands_type const operands_type_table[(int)opcode::num_opcodes] = {
    operands_type::none, // not_opcode
    operands_type::none, // NO_OPCODE
    operands_type::START,  // START
    operands_type::none,  // END
    operands_type::DS,  // DS
    operands_type::DC, // DC

    operands_type::INOUT, // IN
    operands_type::INOUT, // OUT
    operands_type::none, // RPUSH
    operands_type::none, // RPOP

    operands_type::register_register, // LD
    operands_type::register_address, // LD_ADR
    operands_type::register_address, // ST
    operands_type::register_address, // LAD

    operands_type::register_register, // ADDA
    operands_type::register_address, // ADDA_ADR
    operands_type::register_register, // ADDL
    operands_type::register_address, // ADDL_ADR
    operands_type::register_register, // SUBA
    operands_type::register_address, // SUBA_ADR
    operands_type::register_register, // SUBL
    operands_type::register_address, // SUBL_ADR
    operands_type::register_register, // AND
    operands_type::register_address, // AND_ADR
    operands_type::register_register, // OR
    operands_type::register_address, // OR_ADR
    operands_type::register_register, // XOR
    operands_type::register_address, // XOR_ADR

    operands_type::register_register, // CPA
    operands_type::register_address, // CPA_ADR
    operands_type::register_register, // CPL
    operands_type::register_address, // CPL_ADR

    operands_type::register_address, // SLA
    operands_type::register_address, // SRA
    operands_type::register_address, // SLL
    operands_type::register_address, // SRL

    operands_type::address, // JPL
    operands_type::address, // JMI
    operands_type::address, // JNZ
    operands_type::address, // JZE
    operands_type::address, // JOV
    operands_type::address, // JUMP
    operands_type::address, // PUSH
    operands_type::register_, // POP

    operands_type::address, // CALL
    operands_type::none, // RET

    operands_type::address, // SVC
    operands_type::none, // NOP
    operands_type::none, // HLT
    operands_type::none, // DATA
};

operation::operation()
    : opcode_(opcode::not_opcode)
    , line_(0)
{}

size_t operation::get_code_length() {
    switch (opcode_) {
        case opcode::START:
        case opcode::END:
        case opcode::DS:
        case opcode::DC:
        case opcode::IN:
        case opcode::OUT:
        case opcode::RPUSH:
        case opcode::RPOP:
        case opcode::NO_OPCODE:
            return 0;

        case opcode::LD:
        case opcode::ADDA:
        case opcode::ADDL:
        case opcode::SUBA:
        case opcode::SUBL:
        case opcode::AND:
        case opcode::OR:
        case opcode::XOR:
        case opcode::CPA:
        case opcode::CPL:
        case opcode::POP:
        case opcode::RET:
        case opcode::NOP:
        case opcode::PUSHF:
        case opcode::POPF:
        case opcode::RETI:
        case opcode::HLT:
        case opcode::DATA:
            return 1;

        case opcode::LD_ADR:
        case opcode::ST:
        case opcode::LAD:
        case opcode::ADDA_ADR:
        case opcode::ADDL_ADR:
        case opcode::SUBA_ADR:
        case opcode::SUBL_ADR:
        case opcode::AND_ADR:
        case opcode::OR_ADR:
        case opcode::XOR_ADR:
        case opcode::CPA_ADR:
        case opcode::CPL_ADR:
        case opcode::SLA:
        case opcode::SLL:
        case opcode::SRA:
        case opcode::SRL:
        case opcode::JPL:
        case opcode::JMI:
        case opcode::JNZ:
        case opcode::JZE:
        case opcode::JOV:
        case opcode::JUMP:
        case opcode::PUSH:
        case opcode::CALL:
        case opcode::SVC:
            return 2;

        default:
            throw std::runtime_error("内部エラー");
    }
}

bool operation::is_comet2_operation() {
    return get_code_length() != 0;
}
