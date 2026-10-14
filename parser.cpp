//
// Created by kacper on 10/14/26.
//

#include "parser.h"

#include <stdexcept>


enum class Redirect_state {
    None,
    Input,
    Output,
    Append
};

std::vector<command> parser(const std::string &raw_input_data) {
    Redirect_state redir_state = Redirect_state::None;

    std::vector<command> temp_commands;
    command temp_command;
    std::string temp_arg;

    bool in_quote = false;
    bool syntax_error = false;

    //Lambda function for pushing arguments, input/output file names
    auto arg_push = [&]() {
        if (redir_state == Redirect_state::Input) {
            temp_command.input_file = temp_arg;
        } else if (redir_state == Redirect_state::Output) {
            temp_command.output_file = temp_arg;
            temp_command.output_mode = Output_mode::Overwrite;
        } else if (redir_state == Redirect_state::Append) {
            temp_command.output_file = temp_arg;
            temp_command.output_mode = Output_mode::Append;
        } else {
            temp_command.args.push_back(temp_arg);
        }
        temp_arg.clear();
        redir_state = Redirect_state::None;
    };

    for (int i = 0; i < raw_input_data.size(); i++) {
        bool hasNext = i + 1 < raw_input_data.size();

        //quote handling
        if (raw_input_data[i] == '"') {
            in_quote = !in_quote;
            continue;
        }

        //text in quote handling
        if (in_quote) {
            temp_arg += raw_input_data[i];
            continue;
        }

        if (raw_input_data[i] == '<') {
            if (!temp_arg.empty()) {
                arg_push();
            }
            redir_state = Redirect_state::Input;
            continue;
        }
        if (raw_input_data[i] == '>') {
            if (!temp_arg.empty()) {
                arg_push();
            }
            if (hasNext && raw_input_data[i + 1] == '>') {
                redir_state = Redirect_state::Append;
                i++;
            } else {
                redir_state = Redirect_state::Output;
            }
            continue;
        }


        if (raw_input_data[i] == ' ') {
            if (!temp_arg.empty()) {
                arg_push();
            }
            continue;
        }
        if (raw_input_data[i] == '|') {
            if (!temp_arg.empty()) {
                arg_push();
            }
            if (redir_state != Redirect_state::None) {
                syntax_error = true;
                break;
            }
            if (!temp_command.args.empty()) {
                temp_commands.push_back(temp_command);
            }
            temp_command = command{};
            temp_arg.clear();
            continue;
        }
        //home handling
        if (raw_input_data[i] == '~' && !in_quote && temp_arg.empty()) {
            if (i + 1 == raw_input_data.size() || raw_input_data[i + 1] == '/' || raw_input_data[i + 1] == ' ' ||
                raw_input_data[i + 1] == '|') {
                char *home = getenv("HOME");
                if (home != nullptr) {
                    temp_arg += home;
                    continue;
                }
            }
        }

        temp_arg += raw_input_data[i];
    }
    if (in_quote) {
        throw std::runtime_error("Quote not closed");
    }
    if (syntax_error) {
        throw std::runtime_error("Syntax error near unexpected token '|'");
    }
    if (!temp_arg.empty()) {
        arg_push();
    }
    if (redir_state != Redirect_state::None) {
        throw std::runtime_error("Expected a string, but found end of the input");
    }
    if (!temp_command.args.empty()) {
        temp_commands.push_back(temp_command);
    }
    return temp_commands;
}
