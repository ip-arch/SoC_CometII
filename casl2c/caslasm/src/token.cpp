#include "token.hpp"

token::token()
    : token_type_(token_type::identifier)
    , integer_value_(0)
    , keyword_(keyword::not_keyword)
    , is_immediate_(false)
    , line_(0)
    , column_(0)
{}

bool token::is_register(bool is_compiled) {
    if (is_compiled && keyword::GR0 <= keyword_ && keyword_ <= keyword::GR15) {
        return true;
    }

    if (!is_compiled && keyword::GR0 <= keyword_ && keyword_ <= keyword::GR7) {
        return true;
    }

    return false;
}
