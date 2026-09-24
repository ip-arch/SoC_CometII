#include <algorithm>
#include <cctype>
#include <vector>

#include <error.hpp>
#include <operation.hpp>

bool is_valid_label(std::string label, bool is_compiled) {
    if (label.size() == 0) {
        return false;
    }

    if (!is_compiled && 8 < label.size()) {
        return false;
    }

    for (size_t i = 0; i < label.size(); ++i) {
        char c = label[i];

        if (!is_compiled && (c == '@' || c == '_' || std::islower(c))) {
            return false;
        }

        if (is_compiled && i != 0 && c == '@') {
            return false;
        }
    }

    return true;
}

// 無効な命令を検出し、見つかったエラーを返す。
std::vector<std::string> check(std::vector<operation> operations, bool is_compiled) {
    std::vector<std::string> errors;

    bool START_found = false;
    bool END_found = false;
    bool label_only = false;

    for (operation op : operations) {
        if (op.label_.string_value_.size() > 0 && !is_valid_label(op.label_.string_value_, is_compiled)) {
            errors.push_back(make_error_message(op.label_.line_, op.label_.column_, "無効なラベルです。"));
        }

        if (END_found) {
            errors.push_back(make_error_message(op.line_, "END命令より後に命令があります。"));
        }

        if (op.opcode_ == opcode::START) {
            if (START_found) {
                errors.push_back(make_error_message(op.line_, "複数のSTART命令があります。"));
            } else {
                START_found = true;

                if (op.label_.string_value_.size() == 0) {
                    errors.push_back(make_error_message(op.line_, "START命令にはラベルが必要です。"));
                }
            }
        }

        if (op.opcode_ == opcode::not_opcode) {
            errors.push_back(make_error_message(op.line_, "無効な命令コードです。"));
        }

        if (!is_compiled) {
            switch (op.opcode_) {
                case opcode::PUSHF:
                case opcode::POPF:
                case opcode::RETI:
                case opcode::HLT:
                    errors.push_back(make_error_message(op.line_, "無効な命令コードです。"));
                    break;

                default:
                    break;
            }
        }

        if (op.opcode_ == opcode::END) {
            END_found = true;
            if (!op.label_.string_value_.empty())
                errors.push_back(make_error_message(op.line_, "END命令にはラベルを定義できません。"));
            if (label_only)
                errors.push_back(make_error_message(op.line_, "END命令の直前にラベルだけの行があります。"));
        }

        label_only = (op.opcode_ == opcode::NO_OPCODE);

        switch (operands_type_table[(size_t)op.opcode_]) {
            case operands_type::none:
                if (op.operands_.size() != 0) {
                    errors.push_back(make_error_message(op.line_, "オペランドの数が正しくありません。"));
                }
                break;

            case operands_type::register_:
                if (op.operands_.size() != 1) {
                    errors.push_back(make_error_message(op.line_, "オペランドの数が正しくありません。"));
                    break;
                }

                if (!op.operands_[0].is_register(is_compiled)) {
                    errors.push_back(make_error_message(op.operands_[0].line_, op.operands_[0].column_, "無効なGRです。"));
                }
                break;

            case operands_type::address:
                if (!(1 <= op.operands_.size() && op.operands_.size() <= 2)) {
                    errors.push_back(make_error_message(op.line_, "オペランドの数が正しくありません。"));
                    break;
                }

                if (op.operands_[0].token_type_ == token_type::string_constant && !op.operands_[0].is_immediate_) {
                    errors.push_back(make_error_message(op.operands_[0].line_, op.operands_[0].column_, "10進定数、16進定数、アドレス定数又はリテラルが必要です。"));
                }

                if (op.operands_.size() > 1 && !op.operands_[1].is_register(is_compiled)) {
                    errors.push_back(make_error_message(op.operands_[1].line_, op.operands_[1].column_, "無効なGRです。"));
                }
                break;

            case operands_type::register_register:
                if (op.operands_.size() != 2) {
                    errors.push_back(make_error_message(op.line_, "オペランドの数が正しくありません。"));
                    break;
                }

                for (token operand : op.operands_) {
                    if (!operand.is_register(is_compiled)) {
                        errors.push_back(make_error_message(operand.line_, operand.column_, "無効なGRです。"));
                    }
                }
                break;

            case operands_type::register_address:
                if (!(2 <= op.operands_.size() && op.operands_.size() <= 3)) {
                    errors.push_back(make_error_message(op.line_, "オペランドの数が正しくありません。"));
                    break;
                }

                if (!op.operands_[0].is_register(is_compiled)) {
                    errors.push_back(make_error_message(op.operands_[0].line_, op.operands_[0].column_, "無効なGRです。"));
                }

                if (op.operands_[1].token_type_ == token_type::string_constant && !op.operands_[1].is_immediate_) {
                    errors.push_back(make_error_message(op.operands_[1].line_, op.operands_[1].column_, "10進定数、16進定数、アドレス定数又はリテラルが必要です。"));
                }

                if (op.operands_.size() > 2 && (!op.operands_[2].is_register(is_compiled) || op.operands_[2].keyword_ == keyword::GR0)) {
                    errors.push_back(make_error_message(op.operands_[2].line_, op.operands_[2].column_, "無効なGRです。"));
                }
                break;

            case operands_type::START:
                if (!(0 <= op.operands_.size() && op.operands_.size() <= 1)) {
                    errors.push_back(make_error_message(op.line_, "オペランドの数が正しくありません。"));
                    break;
                }

                if (op.operands_.size() > 0 && !is_valid_label(op.operands_[0].string_value_, is_compiled)) {
                    errors.push_back(make_error_message(op.operands_[0].line_, op.operands_[0].column_, "無効なラベルです。"));
                }
                break;

            case operands_type::DS:
                if (op.operands_.size() != 1) {
                    errors.push_back(make_error_message(op.line_, "オペランドの数が正しくありません。"));
                    break;
                }

                if (op.operands_[0].token_type_ != token_type::integer_constant || op.operands_[0].is_immediate_ || op.operands_[0].integer_value_ < 0) {
                    errors.push_back(make_error_message(op.operands_[0].line_, op.operands_[0].column_, "0以上の10進定数が必要です。"));
                }
                break;

            case operands_type::DC:
                if (op.operands_.size() == 0) {
                    errors.push_back(make_error_message(op.line_, "オペランドの数が正しくありません。"));
                    break;
                }

                for (token operand : op.operands_) {
                    if (operand.is_immediate_ || operand.is_register(is_compiled)) {
                        errors.push_back(make_error_message(operand.line_, operand.column_, "10進定数、16進定数、文字定数又はアドレス定数が必要です。"));
                    }
                }
                break;

            case operands_type::INOUT:
                if (op.operands_.size() != 2) {
                    errors.push_back(make_error_message(op.line_, "オペランドの数が正しくありません。"));
                    break;
                }

                for (token operand : op.operands_) {
                    if (!is_valid_label(operand.string_value_, is_compiled)) {
                        errors.push_back(make_error_message(operand.line_, operand.column_, "無効なラベルです。"));
                    }
                }
                break;
        }
    }

    if (!START_found) {
        errors.push_back("エラー: START命令が見つかりません。");
    }

    if (!END_found) {
        errors.push_back("エラー: END命令が見つかりません。");
    }

    return errors;
}
