#include <vector>

#include "commandline_option.hpp"

commandline_option::commandline_option() : compiled_(false){}

commandline_option parse_commandline_option(std::vector<std::string> args) {
    commandline_option option;

    bool at_output_path = false;

    for (std::string arg : args) {
        if (at_output_path) {
            at_output_path = false;
            option.output_path_ = arg;
        } else {
            if (arg == "-compiled") {
                option.compiled_ = true;
            } else if (arg == "-o") {
                at_output_path = true;
            } else {
                option.input_path_ = arg;
            }
        }
    }

    return option;
}
