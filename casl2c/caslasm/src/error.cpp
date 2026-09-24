#include <string>

#include "error.hpp"

// 統一したエラーメッセージを作るための便利関数
std::string make_error_message(int line, int column, std::string message) {
    std::ostringstream output_string_stream;
    output_string_stream << line << "行: " << column << "列: エラー: " << message;
    return output_string_stream.str();
}

// 同上
std::string make_error_message(int line, std::string message) {
    std::ostringstream output_string_stream;
    output_string_stream << line << "行: エラー: " << message;
    return output_string_stream.str();
}
