#include <iostream>
#include <fstream>
#include <iterator>
#include <algorithm>
#include <vector>
#include <cstdlib>
#include <iomanip>

#include "commandline_option.hpp"
#include "memory_base.hpp"

#include "lex.hpp"
#include "parse.hpp"
#include "distinguish_operation.hpp"
#include "check.hpp"
#include "expand_macro_operation.hpp"
#include "expand_literal.hpp"
#include "add_memory_image.hpp"
#include "resolve_address.hpp"
#include "generate_target_code.hpp"

int main(int argc, char* argv[]) {
    try {
        if (argc == 1) {
            std::cerr << "使い方: caslasm <CASL2ファイル> -o <出力ファイル>" << std::endl;
            std::exit(EXIT_FAILURE);
        }

        commandline_option option = parse_commandline_option(std::vector<std::string>(argv + 1, argv + argc));
        // 出力ファイル名の指定がないとき、入力がファイルなら、その拡張子をhexにしたものを出力ファイルとする。
        // 入力が標準入力ならエラー。
        if (option.output_path_.size() == 0) {
            if (option.input_path_ != "-")  {
                int len = option.input_path_.rfind('.');
                option.output_path_ = option.input_path_.substr(0,len) + ".hex";
            }
            else  {
                std::cerr << "エラー: 出力ファイル名が指定されていません。" << std::endl;
                std::exit(EXIT_FAILURE);
            }
        }

        // 入力ファイル名が - のときは標準入力から読む。
        // 標準入力または入力ファイルのストリームバッファを取り出す。
        std::streambuf* isbuf;
        std::ifstream file_input_stream;
        if (option.input_path_ == "-")
            isbuf = std::cin.rdbuf();
        else  {
            file_input_stream.open(option.input_path_);
            if (!file_input_stream) {
                std::cerr << "エラー: " << option.input_path_ << " が開けません。" << std::endl;
                std::exit(EXIT_FAILURE);
            }
            isbuf = file_input_stream.rdbuf();
        }

        memory_base base;
        base.code_section_base_     = 1024 * 0;
        base.data_section_base_     = 1024 * 4;
        //base.interrupt_vector_base_ = 1024 * 2;
        base.stack_top_             = 1024 * 8;

        std::string source_code((std::istreambuf_iterator<char>(isbuf)), std::istreambuf_iterator<char>());
        std::string startup_source_code(
#include "startup.cas"
        );

        std::vector<operation> operations = distinguish_operation(parse(lex(source_code), option.compiled_), option.compiled_);

        std::vector<std::string> errors = check(operations, option.compiled_);

        if (errors.size() > 0) {
            for (std::string error : errors) {
                std::cerr << error << std::endl;
            }

            std::exit(EXIT_FAILURE);
        }

        std::vector<operation> startup_operations = distinguish_operation(parse(lex(startup_source_code), true), true);
        operations.insert(operations.begin(), startup_operations.begin(), startup_operations.end());

        std::vector<uint16_t> target_code = generate_target_code(resolve_address(add_memory_image(expand_macro_operation(expand_literal(operations)), base, option.compiled_), base, option.compiled_));

        std::ofstream file_output_stream(option.output_path_, std::ios::binary);

        if (!file_output_stream) {
            std::cerr << "エラー: " << option.output_path_ << " が開けません。" << std::endl;
            std::exit(EXIT_FAILURE);
        }

        for (uint16_t word : target_code) {
            file_output_stream << std::setfill('0') << std::setw(4) << std::hex << std::uppercase << word << std::endl;
        }

        file_output_stream << std::flush;

        std::exit(EXIT_SUCCESS);
    } catch (const std::runtime_error& error) {
        std::cerr << error.what() << std::endl;
        std::exit(EXIT_FAILURE);
    }
}
