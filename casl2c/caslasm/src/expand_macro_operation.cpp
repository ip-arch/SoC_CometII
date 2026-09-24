#include <vector>
#include <sstream>

#include "operation.hpp"

#include "lex.hpp"
#include "parse.hpp"

// マクロ命令を展開する。
std::vector<operation> expand_macro_operation(std::vector<operation> operations) {
    std::vector<operation> operations_without_macros;

    for (operation op : operations) {
        switch (op.opcode_) {
            case opcode::START:
                op.label_.string_value_ = "@@start";

                if (op.operands_.size() == 0) {
                    op.opcode_ = opcode::NO_OPCODE;
                } else {
                    op.opcode_ = opcode::JUMP;
                }

                operations_without_macros.push_back(op);

                break;

            case opcode::IN:
            case opcode::OUT: {
                std::ostringstream expanded_source_code_stream;

                expanded_source_code_stream << op.label_.string_value_ <<
                    " PUSH 0,GR1\n"
                    " PUSH 0,GR2\n"
                    " LAD GR1," << op.operands_[0].string_value_ << "\n"
                    " LAD GR2," << op.operands_[1].string_value_ << "\n"
                    " SVC " << (op.opcode_ == opcode::IN ? '1' : '2') << "\n"
                    " POP GR2\n"
                    " POP GR1";

                for (operation expanded_operation : parse(lex(expanded_source_code_stream.str()), true)) {
                    operations_without_macros.push_back(expanded_operation);
                }

                break;
            }

            case opcode::RPUSH: {
                std::ostringstream expanded_source_code_stream;
                expanded_source_code_stream << op.label_.string_value_ << " LAD GR15,-1,GR15" << "\n";
                expanded_source_code_stream                            << " ST GR0,0,GR15" << "\n";

                for (int i = 1; i <= 14; ++i) {
                    expanded_source_code_stream << " PUSH 0,GR" << i << "\n";
                }

                for (operation expanded_operation : parse(lex(expanded_source_code_stream.str()), true)) {
                    operations_without_macros.push_back(expanded_operation);
                }

                break;
            }

            case opcode::RPOP: {
                std::ostringstream expanded_source_code_stream;
                for (int i = 14; i >= 0; --i) {

                    if (i == 14) {
                        expanded_source_code_stream << op.label_.string_value_ << " POP GR" << i << "\n";
                    } else {
                        expanded_source_code_stream                            << " POP GR" << i << "\n";
                    }
                }

                for (operation expanded_operation : parse(lex(expanded_source_code_stream.str()), true)) {
                    operations_without_macros.push_back(expanded_operation);
                }

                break;
            }

            default:
                operations_without_macros.push_back(op);
        }
    }

    return operations_without_macros;
}
