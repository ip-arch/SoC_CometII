#pragma once

#include <vector>

#include "token.hpp"
#include "operation.hpp"

std::vector<operation> parse(std::vector<token> tokens, bool is_compiled);
