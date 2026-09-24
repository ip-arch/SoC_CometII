#pragma once

#include <string>
#include <vector>

struct commandline_option {
    std::string input_path_;
    std::string output_path_;
    bool compiled_;

    commandline_option();
};

commandline_option parse_commandline_option(std::vector<std::string> args);
