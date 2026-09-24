#pragma once

#include <vector>

#include "operation.hpp"
#include "memory_base.hpp"

std::vector<operation> add_memory_image(std::vector<operation> operations, memory_base base, bool is_compiled);
