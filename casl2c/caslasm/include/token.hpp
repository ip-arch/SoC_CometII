#pragma once

#include <string>

enum class keyword {
    not_keyword,
    GR0,
    GR1,
    GR2,
    GR3,
    GR4,
    GR5,
    GR6,
    GR7,
    GR8,
    GR9,
    GR10,
    GR11,
    GR12,
    GR13,
    GR14,
    GR15,
};

enum class token_type {
    identifier,
    string_constant,
    integer_constant,
    comma,
    comment,
    newline,
    whitespace,
    equal,
};

enum class newline_type {
  newline,
  with_whitespace,
  with_semicolon,
  with_whitespace_semicolon,
};

struct token {
    token_type token_type_;

    int integer_value_;
    std::string string_value_;
    newline_type newline_type_;

    keyword keyword_;
    bool is_immediate_;

    int line_;
    int column_;

    token();

    bool is_register(bool is_compiled);
};
