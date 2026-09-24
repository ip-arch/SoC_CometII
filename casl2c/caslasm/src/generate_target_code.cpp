#include <stdexcept>
#include <vector>

#include "operation.hpp"

size_t get_gr_number(token gr_token) {
    return (size_t)(gr_token.keyword_) - 1;
}

// 目的コードを生成する。
std::vector<uint16_t> generate_target_code(std::vector<operation> operations) {
    std::vector<uint16_t> target_code;

    for (operation op : operations) {
        switch (op.opcode_) {
            case opcode::START:
            case opcode::END:
            case opcode::DS:
            case opcode::DC:
                break;

            case opcode::RET:
                target_code.push_back(0x81 << 8);
                break;

            case opcode::NOP:
                target_code.push_back(0x00 << 8);
                break;

            case opcode::PUSHF:
                target_code.push_back(0x72 << 8);
                break;

            case opcode::POPF:
                target_code.push_back(0x73 << 8);
                break;

            case opcode::RETI:
                target_code.push_back(0xF1 << 8);
                break;

            case opcode::HLT:
                target_code.push_back(0xF2 << 8);
                break;

            case opcode::POP:
                target_code.push_back(0x71 << 8 | get_gr_number(op.operands_[0]) << 4);
                break;

            case opcode::LD:
                target_code.push_back(0x14 << 8 | get_gr_number(op.operands_[0]) << 4 | get_gr_number(op.operands_[1]));
                break;

            case opcode::ADDA:
                target_code.push_back(0x24 << 8 | get_gr_number(op.operands_[0]) << 4 | get_gr_number(op.operands_[1]));
                break;

            case opcode::SUBA:
                target_code.push_back(0x25 << 8 | get_gr_number(op.operands_[0]) << 4 | get_gr_number(op.operands_[1]));
                break;

            case opcode::ADDL:
                target_code.push_back(0x26 << 8 | get_gr_number(op.operands_[0]) << 4 | get_gr_number(op.operands_[1]));
                break;

            case opcode::SUBL:
                target_code.push_back(0x27 << 8 | get_gr_number(op.operands_[0]) << 4 | get_gr_number(op.operands_[1]));
                break;

            case opcode::AND:
                target_code.push_back(0x34 << 8 | get_gr_number(op.operands_[0]) << 4 | get_gr_number(op.operands_[1]));
                break;

            case opcode::OR:
                target_code.push_back(0x35 << 8 | get_gr_number(op.operands_[0]) << 4 | get_gr_number(op.operands_[1]));
                break;

            case opcode::XOR:
                target_code.push_back(0x36 << 8 | get_gr_number(op.operands_[0]) << 4 | get_gr_number(op.operands_[1]));
                break;

            case opcode::CPA:
                target_code.push_back(0x44 << 8 | get_gr_number(op.operands_[0]) << 4 | get_gr_number(op.operands_[1]));
                break;

            case opcode::CPL:
                target_code.push_back(0x45 << 8 | get_gr_number(op.operands_[0]) << 4 | get_gr_number(op.operands_[1]));
                break;

            case opcode::JPL:
                target_code.push_back(0x65 << 8 | (op.operands_.size() == 2 ? get_gr_number(op.operands_[1]) : 0));
                target_code.push_back(op.operands_[0].integer_value_);
                break;

            case opcode::JMI:
                target_code.push_back(0x61 << 8 | (op.operands_.size() == 2 ? get_gr_number(op.operands_[1]) : 0));
                target_code.push_back(op.operands_[0].integer_value_);
                break;

            case opcode::JNZ:
                target_code.push_back(0x62 << 8 | (op.operands_.size() == 2 ? get_gr_number(op.operands_[1]) : 0));
                target_code.push_back(op.operands_[0].integer_value_);
                break;

            case opcode::JZE:
                target_code.push_back(0x63 << 8 | (op.operands_.size() == 2 ? get_gr_number(op.operands_[1]) : 0));
                target_code.push_back(op.operands_[0].integer_value_);
                break;

            case opcode::JOV:
                target_code.push_back(0x66 << 8 | (op.operands_.size() == 2 ? get_gr_number(op.operands_[1]) : 0));
                target_code.push_back(op.operands_[0].integer_value_);
                break;

            case opcode::JUMP:
                target_code.push_back(0x64 << 8 | (op.operands_.size() == 2 ? get_gr_number(op.operands_[1]) : 0));
                target_code.push_back(op.operands_[0].integer_value_);
                break;

            case opcode::PUSH:
                target_code.push_back(0x70 << 8 | (op.operands_.size() == 2 ? get_gr_number(op.operands_[1]) : 0));
                target_code.push_back(op.operands_[0].integer_value_);
                break;

            case opcode::CALL:
                target_code.push_back(0x80 << 8 | (op.operands_.size() == 2 ? get_gr_number(op.operands_[1]) : 0));
                target_code.push_back(op.operands_[0].integer_value_);
                break;

            case opcode::SVC:
                target_code.push_back(0xF0 << 8 | (op.operands_.size() == 2 ? get_gr_number(op.operands_[1]) : 0));
                target_code.push_back(op.operands_[0].integer_value_);
                break;

            case opcode::LD_ADR:
                target_code.push_back(0x10 << 8 | get_gr_number(op.operands_[0]) << 4 | (op.operands_.size() == 3 ? get_gr_number(op.operands_[2]) : 0));
                target_code.push_back(op.operands_[1].integer_value_);
                break;

            case opcode::ST:
                target_code.push_back(0x11 << 8 | get_gr_number(op.operands_[0]) << 4 | (op.operands_.size() == 3 ? get_gr_number(op.operands_[2]) : 0));
                target_code.push_back(op.operands_[1].integer_value_);
                break;

            case opcode::LAD:
                target_code.push_back(0x12 << 8 | get_gr_number(op.operands_[0]) << 4 | (op.operands_.size() == 3 ? get_gr_number(op.operands_[2]) : 0));
                target_code.push_back(op.operands_[1].integer_value_);
                break;

            case opcode::ADDA_ADR:
                target_code.push_back(0x20 << 8 | get_gr_number(op.operands_[0]) << 4 | (op.operands_.size() == 3 ? get_gr_number(op.operands_[2]) : 0));
                target_code.push_back(op.operands_[1].integer_value_);
                break;

            case opcode::SUBA_ADR:
                target_code.push_back(0x21 << 8 | get_gr_number(op.operands_[0]) << 4 | (op.operands_.size() == 3 ? get_gr_number(op.operands_[2]) : 0));
                target_code.push_back(op.operands_[1].integer_value_);
                break;

            case opcode::ADDL_ADR:
                target_code.push_back(0x22 << 8 | get_gr_number(op.operands_[0]) << 4 | (op.operands_.size() == 3 ? get_gr_number(op.operands_[2]) : 0));
                target_code.push_back(op.operands_[1].integer_value_);
                break;

            case opcode::SUBL_ADR:
                target_code.push_back(0x23 << 8 | get_gr_number(op.operands_[0]) << 4 | (op.operands_.size() == 3 ? get_gr_number(op.operands_[2]) : 0));
                target_code.push_back(op.operands_[1].integer_value_);
                break;

            case opcode::AND_ADR:
                target_code.push_back(0x30 << 8 | get_gr_number(op.operands_[0]) << 4 | (op.operands_.size() == 3 ? get_gr_number(op.operands_[2]) : 0));
                target_code.push_back(op.operands_[1].integer_value_);
                break;

            case opcode::OR_ADR:
                target_code.push_back(0x31 << 8 | get_gr_number(op.operands_[0]) << 4 | (op.operands_.size() == 3 ? get_gr_number(op.operands_[2]) : 0));
                target_code.push_back(op.operands_[1].integer_value_);
                break;

            case opcode::XOR_ADR:
                target_code.push_back(0x32 << 8 | get_gr_number(op.operands_[0]) << 4 | (op.operands_.size() == 3 ? get_gr_number(op.operands_[2]) : 0));
                target_code.push_back(op.operands_[1].integer_value_);
                break;

            case opcode::CPA_ADR:
                target_code.push_back(0x40 << 8 | get_gr_number(op.operands_[0]) << 4 | (op.operands_.size() == 3 ? get_gr_number(op.operands_[2]) : 0));
                target_code.push_back(op.operands_[1].integer_value_);
                break;

            case opcode::CPL_ADR:
                target_code.push_back(0x41 << 8 | get_gr_number(op.operands_[0]) << 4 | (op.operands_.size() == 3 ? get_gr_number(op.operands_[2]) : 0));
                target_code.push_back(op.operands_[1].integer_value_);
                break;

            case opcode::SLA:
                target_code.push_back(0x50 << 8 | get_gr_number(op.operands_[0]) << 4 | (op.operands_.size() == 3 ? get_gr_number(op.operands_[2]) : 0));
                target_code.push_back(op.operands_[1].integer_value_);
                break;

            case opcode::SLL:
                target_code.push_back(0x52 << 8 | get_gr_number(op.operands_[0]) << 4 | (op.operands_.size() == 3 ? get_gr_number(op.operands_[2]) : 0));
                target_code.push_back(op.operands_[1].integer_value_);
                break;

            case opcode::SRA:
                target_code.push_back(0x51 << 8 | get_gr_number(op.operands_[0]) << 4 | (op.operands_.size() == 3 ? get_gr_number(op.operands_[2]) : 0));
                target_code.push_back(op.operands_[1].integer_value_);
                break;

            case opcode::SRL:
                target_code.push_back(0x53 << 8 | get_gr_number(op.operands_[0]) << 4 | (op.operands_.size() == 3 ? get_gr_number(op.operands_[2]) : 0));
                target_code.push_back(op.operands_[1].integer_value_);
                break;

            case opcode::DATA:
                target_code.push_back(op.operands_[0].integer_value_);
                break;

            case opcode::NO_OPCODE:
                break;

            default:
                throw std::runtime_error("内部エラー");
        }
    }

    return target_code;
}
