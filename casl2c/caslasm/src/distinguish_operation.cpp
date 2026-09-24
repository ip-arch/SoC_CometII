#include <vector>

#include "token.hpp"
#include "operation.hpp"

// 同じ命令コードでオペランドの書き方が違う命令について、
// オペランドの書き方の違いを反映してそれぞれ別のopcodeを割り当てる。
std::vector<operation> distinguish_operation(std::vector<operation> operations, bool is_compiled) {
    for (operation& op : operations) {
        switch (op.opcode_) {
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

                if (op.operands_.size() > 1 && !op.operands_[1].is_register(is_compiled)) {
                    op.opcode_ = (opcode)((size_t)op.opcode_ + 1);
                }
                break;

            default:
                break;
        }
    }

    return operations;
}
