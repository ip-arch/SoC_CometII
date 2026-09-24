#include <vector>
#include <sstream>
#include <utility>
#include <cctype>
#include <stdexcept>

#include "token.hpp"
#include "operation.hpp"
#include "error.hpp"

opcode string_to_opcode(std::string opcode_string) {
         if (opcode_string == "START") return opcode::START;
    else if (opcode_string == "END")   return opcode::END;
    else if (opcode_string == "DS")    return opcode::DS;
    else if (opcode_string == "DC")    return opcode::DC;
    else if (opcode_string == "IN")    return opcode::IN;
    else if (opcode_string == "OUT")   return opcode::OUT;
    else if (opcode_string == "RPUSH") return opcode::RPUSH;
    else if (opcode_string == "RPOP")  return opcode::RPOP;
    else if (opcode_string == "LD")    return opcode::LD;
    else if (opcode_string == "ST")    return opcode::ST;
    else if (opcode_string == "LAD")   return opcode::LAD;
    else if (opcode_string == "ADDA")  return opcode::ADDA;
    else if (opcode_string == "ADDL")  return opcode::ADDL;
    else if (opcode_string == "SUBA")  return opcode::SUBA;
    else if (opcode_string == "SUBL")  return opcode::SUBL;
    else if (opcode_string == "AND")   return opcode::AND;
    else if (opcode_string == "OR")    return opcode::OR;
    else if (opcode_string == "XOR")   return opcode::XOR;
    else if (opcode_string == "CPA")   return opcode::CPA;
    else if (opcode_string == "CPL")   return opcode::CPL;
    else if (opcode_string == "SLA")   return opcode::SLA;
    else if (opcode_string == "SRA")   return opcode::SRA;
    else if (opcode_string == "SLL")   return opcode::SLL;
    else if (opcode_string == "SRL")   return opcode::SRL;
    else if (opcode_string == "JPL")   return opcode::JPL;
    else if (opcode_string == "JMI")   return opcode::JMI;
    else if (opcode_string == "JNZ")   return opcode::JNZ;
    else if (opcode_string == "JZE")   return opcode::JZE;
    else if (opcode_string == "JOV")   return opcode::JOV;
    else if (opcode_string == "JUMP")  return opcode::JUMP;
    else if (opcode_string == "PUSH")  return opcode::PUSH;
    else if (opcode_string == "POP")   return opcode::POP;
    else if (opcode_string == "CALL")  return opcode::CALL;
    else if (opcode_string == "RET")   return opcode::RET;
    else if (opcode_string == "SVC")   return opcode::SVC;
    else if (opcode_string == "NOP")   return opcode::NOP;
    else if (opcode_string == "PUSHF")   return opcode::PUSHF;
    else if (opcode_string == "POPF")   return opcode::POPF;
    else if (opcode_string == "RETI")   return opcode::RETI;
    else if (opcode_string == "HLT")   return opcode::HLT;
    else if (opcode_string == "DATA")  return opcode::DATA;
    else                               return opcode::not_opcode;
}

// 構文解析を行う。
std::vector<operation> parse(std::vector<token> tokens, bool is_compiled) {
    std::vector<operation> operations;

    for (size_t i = 0; i < tokens.size(); ++i) {
        if (tokens[i].token_type_ == token_type::newline) {
            continue;
        }

        operation parsed_operation;

        if (tokens[i].token_type_ == token_type::identifier) {
            parsed_operation.label_ = tokens[i];
            ++i;
        } else {
            token empty_identifier_token;
            empty_identifier_token.token_type_ = token_type::identifier;

            parsed_operation.label_ = empty_identifier_token;
        }

        if (tokens[i].token_type_ == token_type::whitespace) {
            ++i;
        } else if (!parsed_operation.label_.string_value_.empty() && tokens[i].token_type_ == token_type::newline && tokens[i].newline_type_ != newline_type::with_semicolon && is_compiled) {
            parsed_operation.opcode_ = opcode::NO_OPCODE;
            parsed_operation.line_ = tokens[i].line_;

            operations.push_back(parsed_operation);

            continue;
        } else {
            throw std::runtime_error(make_error_message(tokens[i].line_, tokens[i].column_, "空白が必要です。"));
        }

        if (tokens[i].token_type_ == token_type::identifier) {
            parsed_operation.opcode_ = string_to_opcode(tokens[i].string_value_);
            parsed_operation.line_ = tokens[i].line_;
            ++i;
        } else {
            throw std::runtime_error(make_error_message(tokens[i].line_, tokens[i].column_, "命令コードが必要です。"));
        }

        if (tokens[i].token_type_ == token_type::newline && tokens[i].newline_type_ != newline_type::with_semicolon) {
            operations.push_back(parsed_operation);

            continue;
        }

        if (tokens[i].token_type_ == token_type::whitespace) {
            ++i;
        } else {
            throw std::runtime_error(make_error_message(tokens[i].line_, tokens[i].column_, "空白が必要です。"));
        }

        bool first = true;
        for (; tokens[i].token_type_ != token_type::newline && tokens[i].token_type_ != token_type::whitespace; ++i) {
            if (first) {
                first = false;
            } else {
                if (tokens[i].token_type_ == token_type::comma) {
                    ++i;
                } else {
                    throw std::runtime_error(make_error_message(tokens[i].line_, tokens[i].column_, "','が必要です。"));
                }
            }

            bool is_immediate = false;
            if (tokens[i].token_type_ == token_type::equal) {
                is_immediate = true;
                ++i;
            }

            if (!is_immediate && tokens[i].token_type_ != token_type::integer_constant && tokens[i].token_type_ != token_type::string_constant && tokens[i].token_type_ != token_type::identifier) {
                throw std::runtime_error(make_error_message(tokens[i].line_, tokens[i].column_, "10進定数、16進定数、文字定数、アドレス定数、またはリテラルが必要です。"));
            }

            if (is_immediate && tokens[i].token_type_ != token_type::integer_constant && tokens[i].token_type_ != token_type::string_constant) {
                throw std::runtime_error(make_error_message(tokens[i].line_, tokens[i].column_, "10進定数、16進定数、または文字定数が必要です。"));
            }

            if (is_immediate) {
                --tokens[i].column_;
                tokens[i].is_immediate_ = true;
            }

            parsed_operation.operands_.push_back(tokens[i]);
        }

        if (tokens[i].token_type_ == token_type::newline && tokens[i].newline_type_ == newline_type::with_semicolon) {
            throw std::runtime_error(make_error_message(tokens[i].line_, tokens[i].column_, "空白が必要です。"));
        }

        if (tokens[i].token_type_ == token_type::whitespace) {
            for (; tokens[i].token_type_ != token_type::newline; ++i) {}
        }

        operations.push_back(parsed_operation);
    }

    return operations;
}
