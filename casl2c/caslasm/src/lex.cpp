#include <vector>
#include <string>
#include <cctype>
#include <sstream>
#include <stdexcept>

#include "token.hpp"
#include "error.hpp"

const int lookahead_num = 2;
const int eof = -1;

keyword string_to_keyword(std::string keyword_string) {
         if (keyword_string == "GR0")  return keyword::GR0;
    else if (keyword_string == "GR1")  return keyword::GR1;
    else if (keyword_string == "GR2")  return keyword::GR2;
    else if (keyword_string == "GR3")  return keyword::GR3;
    else if (keyword_string == "GR4")  return keyword::GR4;
    else if (keyword_string == "GR5")  return keyword::GR5;
    else if (keyword_string == "GR6")  return keyword::GR6;
    else if (keyword_string == "GR7")  return keyword::GR7;
    else if (keyword_string == "GR8")  return keyword::GR8;
    else if (keyword_string == "GR9")  return keyword::GR9;
    else if (keyword_string == "GR10") return keyword::GR10;
    else if (keyword_string == "GR11") return keyword::GR11;
    else if (keyword_string == "GR12") return keyword::GR12;
    else if (keyword_string == "GR13") return keyword::GR13;
    else if (keyword_string == "GR14") return keyword::GR14;
    else if (keyword_string == "GR15") return keyword::GR15;
    else                               return keyword::not_keyword;
}

enum class state {
    neutral,
    identifier,
    string_constant,
    decimal_constant,
    hexadecimal_constant,
    whitespace,
    comment,
    semicolon,
    whitespace_semicolon,
};

// 字句解析を行う。
std::vector<token> lex(std::string input) {
    std::vector<token> tokens;
    std::vector<char> buffer;
    state current_state = state::neutral;

    int current_line = 1;
    int current_column = 1;

    int token_start_column = 1;

    int lookaheads[lookahead_num] = {};

    bool is_immediate = false;
    for (size_t i = 0; i <= input.size();) {
        for (size_t j = 0; j < lookahead_num; ++j) {
            if (i + j < input.size()) {
                lookaheads[j] = input[i + j];
            } else {
                lookaheads[j] = eof;
            }
        }

        switch (current_state) {
            case state::neutral: {
                switch (lookaheads[0]) {
                    case eof:
                    case '\n': {
                        token newline_token = {};
                        newline_token.token_type_ = token_type::newline;
                        newline_token.newline_type_ = newline_type::newline;
                        newline_token.line_ = current_line;
                        newline_token.column_ = current_column;

                        tokens.push_back(newline_token);

                        ++i;
                        ++current_line;
                        current_column = 1;

                        continue;
                    }

                    case ',': {
                        token comma_token = {};
                        comma_token.token_type_ = token_type::comma;
                        comma_token.line_ = current_line;
                        comma_token.column_ = current_column;

                        tokens.push_back(comma_token);

                        ++i;
                        ++current_column;

                        continue;
                    }

                    case ';': {
                        current_state = state::semicolon;
                        token_start_column = current_column;

                        ++i;
                        ++current_column;

                        continue;
                    }

                    case '\'': {
                        current_state = state::string_constant;
                        token_start_column = current_column;

                        ++i;
                        ++current_column;

                        continue;
                    }

                    case '#': {
                        current_state = state::hexadecimal_constant;
                        token_start_column = current_column;

                        ++i;
                        ++current_column;

                        continue;
                    }

                    case '=': {
                        token equal_token = {};
                        equal_token.token_type_ = token_type::equal;
                        equal_token.line_ = current_line;
                        equal_token.column_ = current_column;

                        tokens.push_back(equal_token);

                        ++i;
                        ++current_column;

                        continue;
                    }

                    case '_':
                    case '@': {
                        current_state = state::identifier;
                        token_start_column = current_column;

                        continue;
                    }
                }

                if ('0' <= lookaheads[0] && lookaheads[0] <= '9') {
                    current_state = state::decimal_constant;
                    token_start_column = current_column;

                    continue;
                }

                if (lookaheads[0] == '-' && '1' <= lookaheads[1] && lookaheads[1] <= '9') {
                    current_state = state::decimal_constant;
                    token_start_column = current_column;

                    buffer.push_back('-');

                    ++i;
                    ++current_column;

                    continue;
                }

                if (('a' <= lookaheads[0] && lookaheads[0] <= 'z') || ('A' <= lookaheads[0] && lookaheads[0] <= 'Z')) {
                    current_state = state::identifier;
                    token_start_column = current_column;

                    continue;
                }

                if (std::isspace(lookaheads[0])) {
                    current_state = state::whitespace;
                    token_start_column = current_column;

                    continue;
                }

                current_state = state::comment;
                token_start_column = current_column;

                continue;
            }

            case state::identifier: {
                if (lookaheads[0] == '@' || lookaheads[0] == '_' || ('a' <= lookaheads[0] && lookaheads[0] <= 'z') || ('A' <= lookaheads[0] && lookaheads[0] <= 'Z') || ('0' <= lookaheads[0] && lookaheads[0] <= '9')) {
                    buffer.push_back(lookaheads[0]);
                    ++i;
                    ++current_column;
                } else {
                    std::string identifier_string(buffer.begin(), buffer.end());

                    token identifier_token = {};
                    identifier_token.token_type_ = token_type::identifier;
                    identifier_token.string_value_ = identifier_string;
                    identifier_token.keyword_ = string_to_keyword(identifier_string);
                    identifier_token.line_ = current_line;
                    identifier_token.column_ = token_start_column;

                    tokens.push_back(identifier_token);

                    buffer.clear();

                    current_state = state::neutral;
                }

                continue;
            }

            case state::string_constant: {
                if (lookaheads[0] == '\'' && lookaheads[1] == '\'') {
                    buffer.push_back('\'');
                    i += 2;
                    current_column += 2;
                } else if (lookaheads[0] == '\'') {
                    std::string string_constant_content(buffer.begin(), buffer.end());

                    token string_constant_token = {};
                    string_constant_token.token_type_ = token_type::string_constant;
                    string_constant_token.string_value_ = string_constant_content;
                    string_constant_token.is_immediate_ = is_immediate;
                    string_constant_token.line_ = current_line;
                    string_constant_token.column_ = token_start_column;

                    tokens.push_back(string_constant_token);

                    buffer.clear();

                    current_state = state::neutral;
                    is_immediate = false;
                    ++i;
                    ++current_column;
                } else if (lookaheads[0] == eof) {
                    throw std::runtime_error(make_error_message(current_line, current_column, "'が必要です。"));
                } else {
                    buffer.push_back(lookaheads[0]);
                    ++i;
                    ++current_column;
                }

                continue;
            }

            case state::decimal_constant: {
                if ('0' <= lookaheads[0] && lookaheads[0] <= '9') {
                    buffer.push_back(lookaheads[0]);
                    ++i;
                    ++current_column;
                } else {
                    int integer_value;
                    std::istringstream input_string_stream(std::string(buffer.begin(), buffer.end()));
                    input_string_stream >> integer_value;

                    token integer_constant_token = {};
                    integer_constant_token.token_type_ = token_type::integer_constant;
                    integer_constant_token.integer_value_ = integer_value;
                    integer_constant_token.is_immediate_ = is_immediate;
                    integer_constant_token.line_ = current_line;
                    integer_constant_token.column_ = token_start_column;

                    tokens.push_back(integer_constant_token);

                    buffer.clear();

                    current_state = state::neutral;
                    is_immediate = false;
                }

                continue;
            }

            case state::hexadecimal_constant: {
                if (('0' <= lookaheads[0] && lookaheads[0] <= '9') || ('A' <= lookaheads[0] && lookaheads[0] <= 'F')) {
                    buffer.push_back(lookaheads[0]);
                    ++i;
                    ++current_column;
                } else {
                    int integer_value;
                    std::istringstream input_string_stream(std::string(buffer.begin(), buffer.end()));
                    input_string_stream >> std::hex >> integer_value;

                    token integer_constant_token = {};
                    integer_constant_token.token_type_ = token_type::integer_constant;
                    integer_constant_token.integer_value_ = integer_value;
                    integer_constant_token.is_immediate_ = is_immediate;
                    integer_constant_token.line_ = current_line;
                    integer_constant_token.column_ = token_start_column;

                    tokens.push_back(integer_constant_token);

                    buffer.clear();

                    current_state = state::neutral;
                    is_immediate = false;
                }

                continue;
            }

            case state::whitespace: {
                if (lookaheads[0] == eof || lookaheads[0] == '\n') {
                    token newline_token = {};
                    newline_token.token_type_ = token_type::newline;
                    newline_token.newline_type_ = newline_type::with_whitespace;
                    newline_token.line_ = current_line;
                    newline_token.column_ = token_start_column;

                    tokens.push_back(newline_token);

                    ++i;
                    ++current_line;
                    current_column = 1;

                    current_state = state::neutral;

                } else if (lookaheads[0] == ';') {
                    ++i;
                    ++current_column;

                    current_state = state::whitespace_semicolon;

                } else if (isspace(lookaheads[0])) {
                    ++i;
                    ++current_column;

                } else {
                    token whitespace_token = {};
                    whitespace_token.token_type_ = token_type::whitespace;
                    whitespace_token.line_ = current_line;
                    whitespace_token.column_ = token_start_column;

                    tokens.push_back(whitespace_token);

                    current_state = state::neutral;
                }

                continue;
            }

            case state::semicolon:
            case state::whitespace_semicolon:
            case state::comment: {
                if (lookaheads[0] == '\n' || lookaheads[0] == eof) {
                    token current_token = {};
                    current_token.line_ = current_line;
                    current_token.column_ = token_start_column;
                    if (current_state == state::comment) {
                        current_token.token_type_ = token_type::comment;
                    } else {
                        current_token.token_type_ = token_type::newline;

                        if (current_state == state::semicolon)
                            current_token.newline_type_ = newline_type::with_semicolon;
                        else
                            current_token.newline_type_ = newline_type::with_whitespace_semicolon;

                        ++i;
                        ++current_line;
                        current_column = 1;
                    }

                    tokens.push_back(current_token);

                    current_state = state::neutral;
                } else {
                    ++i;
                    ++current_column;
                }

                continue;
            }
        }
    }

    return tokens;
}
