#include <vector>
#include <cassert>
#include <sstream>

#include "token.hpp"
#include "operation.hpp"
#include "memory_base.hpp"
#include "lex.hpp"
#include "parse.hpp"

size_t calc_code_length(std::vector<operation> operations) {
    size_t code_length = 0;

    for (operation op : operations) {
        code_length += op.get_code_length();
    }

    return code_length;
}

// メモリイメージとなる命令列を作成し、引数の命令列に追加する。
std::vector<operation> add_memory_image(std::vector<operation> operations, memory_base base, bool is_compiled) {
    int data_base = base.code_section_base_ + calc_code_length(operations) + 4;

    std::ostringstream initial_data_stream;

    initial_data_stream <<
        "@@interrupt_vector_base DATA " << base.interrupt_vector_base_ << "\n"
        "@@stack_top             DATA " << base.stack_top_ << "\n"
        "@@data_base             DATA " << data_base << "\n"
        "@@data_section_base     DATA " << base.data_section_base_;

    std::vector<operation> initial_data = parse(lex(initial_data_stream.str()), true);
    operations.insert(operations.end(), initial_data.begin(), initial_data.end());

    std::vector<operation> data_operations;

    // DC,DS命令をみて、DATA命令を作成する。
    // DATA 0が数値、DATA 1が文字列、DATA 2が領域確保、DATA 3がメモリイメージの終了を表す。
    for (operation op : operations) {
        if (op.opcode_ == opcode::DC) {
            for (token operand : op.operands_) {
                if (operand.token_type_ == token_type::integer_constant) {
                    std::ostringstream integer_constant_data_stream;
                    integer_constant_data_stream <<
                        " DATA 0\n"
                        " DATA " << operand.integer_value_;

                    std::vector<operation> integer_constant_data = parse(lex(integer_constant_data_stream.str()), true);

                    data_operations.insert(data_operations.end(), integer_constant_data.begin(), integer_constant_data.end());
                } else if (operand.token_type_ == token_type::string_constant) {
                    std::ostringstream string_constant_data_stream;
                    int size = operand.string_value_.size();
                    if (is_compiled) {
                        ++size;
                    }
                    string_constant_data_stream <<
                        " DATA 1\n"
                        " DATA " << size << "\n";

                    for (char c : operand.string_value_) {
                        string_constant_data_stream << " DATA " << (int)c << "\n";
                    }
                    if (is_compiled) {
                        string_constant_data_stream << " DATA " << 0 << "\n";
                    }

                    std::vector<operation> string_constant_data = parse(lex(string_constant_data_stream.str()), true);

                    data_operations.insert(data_operations.end(), string_constant_data.begin(), string_constant_data.end());
                } else if (operand.token_type_ == token_type::identifier) {
                    std::ostringstream label_constant_data_stream;
                    label_constant_data_stream <<
                        " DATA 0\n"
                        " DATA " << operand.string_value_;

                    std::vector<operation> label_constant_data = parse(lex(label_constant_data_stream.str()), true);

                    data_operations.insert(data_operations.end(), label_constant_data.begin(), label_constant_data.end());
                }
            }
        } else if (op.opcode_ == opcode::DS && op.operands_[0].integer_value_ > 0) {
            std::ostringstream allocate_data_stream;
            allocate_data_stream <<
                " DATA 2\n"
                " DATA " << op.operands_[0].integer_value_;

            std::vector<operation> allocate_data = parse(lex(allocate_data_stream.str()), true);

            data_operations.insert(data_operations.end(), allocate_data.begin(), allocate_data.end());
        }
    }

    operations.insert(operations.end(), data_operations.begin(), data_operations.end());

    std::vector<operation> data_end_data = parse(lex(" DATA 3"), true);
    operations.insert(operations.end(), data_end_data.begin(), data_end_data.end());

    return operations;
}
