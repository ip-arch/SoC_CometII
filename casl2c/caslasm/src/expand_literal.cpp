#include <sstream>
#include <vector>
#include <map>
#include <stdexcept>

#include "token.hpp"
#include "operation.hpp"

// 命令列からリテラルを探し、そのリテラルに対応するDC命令を作成し、命令列に追加する。
std::vector<operation> expand_literal(std::vector<operation> operations) {
    std::map<int, std::string> integer_literal_label;
    std::map<std::string, std::string> string_literal_label;
    std::vector<operation> dc_operations;
    int counter = 0;

    for (operation op : operations) {
        for (token operand : op.operands_) {
            if (operand.is_immediate_) {
                switch (operand.token_type_) {
                    case token_type::integer_constant: {
                        if (integer_literal_label.find(operand.integer_value_) == integer_literal_label.end()) {
                            std::ostringstream output_string_stream;
                            output_string_stream << "@@literal" << counter;
                            std::string label = output_string_stream.str();

                            ++counter;

                            operation dc_operation;
                            operand.is_immediate_ = false;
                            dc_operation.label_.string_value_ = label;
                            dc_operation.opcode_ = opcode::DC;
                            dc_operation.operands_.push_back(operand);

                            dc_operations.push_back(dc_operation);

                            integer_literal_label[operand.integer_value_] = label;
                        }
                        break;
                    }

                    case token_type::string_constant: {
                        if (string_literal_label.find(operand.string_value_) == string_literal_label.end()) {
                            std::ostringstream output_string_stream;
                            output_string_stream << "@@literal" << counter;
                            std::string label = output_string_stream.str();

                            ++counter;

                            operation dc_operation;
                            operand.is_immediate_ = false;
                            dc_operation.label_.string_value_ = label;
                            dc_operation.opcode_ = opcode::DC;
                            dc_operation.operands_.push_back(operand);

                            dc_operations.push_back(dc_operation);

                            string_literal_label[operand.string_value_] = label;
                        }
                        break;
                    }

                    default:
                        throw std::runtime_error("内部エラー");
                }
            }
        }
    }


    for (operation& op : operations) {
        for (token& operand : op.operands_) {
            if (operand.is_immediate_) {
                switch (operand.token_type_) {
                    case token_type::integer_constant:
                        operand.token_type_ = token_type::identifier;
                        operand.string_value_ = integer_literal_label[operand.integer_value_];

                        break;

                    case token_type::string_constant:
                        operand.token_type_ = token_type::identifier;
                        operand.string_value_ = string_literal_label[operand.string_value_];

                        break;

                    default:
                        throw std::runtime_error("内部エラー");
                }
            }
        }
    }

    operations.insert(operations.end(), dc_operations.begin(), dc_operations.end());

    return operations;
}
