//
// Created by kacper on 10/14/26.
//

#ifndef PIGEONCLI_PARSER_H
#define PIGEONCLI_PARSER_H
#include <string>
#include <vector>

enum class Output_mode {
    None,
    Overwrite,
    Append
};
struct command {
    std::vector<std::string> args;
    std::string input_file;
    std::string output_file;
    Output_mode output_mode = Output_mode::None;
};

std::vector<command> parser(const std::string &raw_input_data);

#endif //PIGEONCLI_PARSER_H
