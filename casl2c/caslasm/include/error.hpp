#pragma once

#include <sstream>

std::string make_error_message(int line, int column, std::string message);
std::string make_error_message(int line, std::string message);
