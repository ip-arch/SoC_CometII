#include <map>
#include <cassert>

#include "error.hpp"
#include "token.hpp"
#include "operation.hpp"
#include "memory_base.hpp"

// オペランドに書かれたラベルを対応するアドレスに書き換える。
std::vector<operation> resolve_address(std::vector<operation> operations, memory_base base, bool is_compiled) {
    std::map<std::string, int> label_address; // ラベルとアドレスの対応表
    std::vector<std::string> labels;

    int code_address = base.code_section_base_;
    int data_address = base.data_section_base_;

    for (operation op : operations) {
        if (op.opcode_ == opcode::NO_OPCODE) {
            if (!op.label_.string_value_.empty()) {
                labels.push_back(op.label_.string_value_);
            }
        } else if (op.opcode_ == opcode::DS) {
            if (!op.label_.string_value_.empty()) {
                labels.push_back(op.label_.string_value_);
            }
            for (std::string lab : labels) {
                label_address[lab] = data_address;
            }
            labels.clear();

            data_address += op.operands_[0].integer_value_;
        } else if (op.opcode_ == opcode::DC) {
            if (!op.label_.string_value_.empty()) {
                labels.push_back(op.label_.string_value_);
            }
            for (std::string lab : labels) {
                label_address[lab] = data_address;
            }
            labels.clear();

            for (token operand : op.operands_) {
                switch (operand.token_type_) {
                    case token_type::identifier:
                    case token_type::integer_constant:
                        data_address += 1;
                        break;

                    case token_type::string_constant:
                        data_address += operand.string_value_.size();
                        if (is_compiled) {
                            data_address += 1;
                        }
                        break;

                    default:
                        assert(false);
                }
            }
        } else {
            if (!op.label_.string_value_.empty()) {
                labels.push_back(op.label_.string_value_);
            }
            for (std::string lab : labels) {
                label_address[lab] = code_address;
            }
            labels.clear();
            code_address += op.get_code_length();
        }
    }

    for (operation& op : operations) {
        for (token& operand : op.operands_) {
            if (operand.keyword_ == keyword::not_keyword && operand.token_type_ == token_type::identifier) {
                std::string label = operand.string_value_;

                try {
                    operand.token_type_ = token_type::integer_constant;
                    operand.integer_value_ = label_address.at(label);
                } catch (std::out_of_range) {
                    throw std::runtime_error(make_error_message(op.operands_[1].line_, op.operands_[1].column_, "ラベル" + label + "が見つかりません"));
                }
            }
        }
    }

    return operations;
}
