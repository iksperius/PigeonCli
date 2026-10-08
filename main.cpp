#include <iostream>
#include <sstream>
#include <unistd.h>
#include <string>
#include <vector>
#include <cstdlib>
#include <sys/wait.h>
#include <fcntl.h>
#include <filesystem>
#include <fstream>
#include <stdexcept>


enum class Redirect_state {
    None,
    Input,
    Output,
    Append
};

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

std::vector<command> parser(std::string raw_input_data);

bool execute(std::vector<command> commands);

int main() {
    std::string raw_input_data;

    while (true) {
        std::cout << ">";
        std::getline(std::cin, raw_input_data);
        std::vector<command> commands;

        try {
            commands = parser(raw_input_data);
            if (commands.empty()) {
                continue;
            }
        } catch (const std::exception &e) {
            std::cout << e.what() << std::endl;
            continue;
        }
        if (!execute(commands)) {
            break;
        }
    }

    return 0;
}


std::vector<command> parser(std::string raw_input_data) {
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

bool execute(std::vector<command> commands) {
    if (commands[0].args[0] == "exit") {
        return false;
    }
    if (commands[0].args[0] == "cd") {
        char *home = getenv("HOME");
        if (commands[0].args.size() > 1) {
            if (chdir(commands[0].args[1].c_str()) == -1) {
                std::cout << "Not a valid directory" << std::endl;
            }
            return true;
        }
        if (home != nullptr) chdir(getenv("HOME"));
        else {
            std::cout << "Home is not set" << std::endl;
        }
        return true;
    }

    //pigeon
    //saving commands
    //arg 0 - command
    //arg 1 - action
    //arg 2 - name

    //TODO:
    // - named/listed variables for commands
    //      - seperate parser and execute to functions
    //      - parse whole command again after pigeon run, apply variables
    if (commands[0].args[0] == "pigeon") {
        std::string line;
        char *home = getenv("HOME");
        if (home == nullptr) {
            std::cout << "Can't open file" << std::endl;
            return true;
        }
        std::string file_location = static_cast<std::string>(home) + "/.config/PigeonCli/saved_commands";
        //trim saved_commands file from path
        std::filesystem::create_directories(std::filesystem::path(file_location).parent_path());

        if (commands[0].args.size() < 2) {
            std::cout << "Usage: pigeon <save|run|list|delete> ..." << std::endl;
            return true;
        }


        if (commands[0].args[1] == "save") {
            if (commands[0].args.size() < 3) {
                std::cout << "Usage: pigeon save <name> \"command\" " << std::endl;
                return true;
            }
            std::vector<std::string> temp_commands_file;
            std::string temp_command = commands[0].args[2] + ":";
            for (int i = 3; i < commands[0].args.size(); i++) {
                temp_command += commands[0].args[i] + ' ';
            }


            //TODO: switch to lambda
            std::ifstream infile(file_location);
            bool replaced = false;
            if (infile.is_open()) {
                while (getline(infile, line)) {
                    size_t col_pos = line.find(':');
                    if (col_pos == std::string::npos) {
                        continue;
                    }
                    if (commands[0].args[2] == line.substr(0, col_pos)) {
                        temp_commands_file.push_back(temp_command);
                        replaced = true;
                        continue;
                    }
                    temp_commands_file.push_back(line);
                }

                infile.close();
            }
            if (!replaced) {
                temp_commands_file.push_back(temp_command);
            }



            std::ofstream outfile(file_location);
            if (!outfile) {
                std::cout << "Can't open file" << std::endl;
                return true;
            }

            for (const auto& l : temp_commands_file) {
                outfile << l << std::endl;
            }
            outfile.close();
            return true;
        }


        if (commands[0].args[1] == "run") {
            if (commands[0].args.size() < 3) {
                std::cout << "Usage: pigeon run <name> \"command\" " << std::endl;
                return true;
            }
            //TODO: switch to lambda
            std::ifstream file(file_location);
            std::string raw_command;
            bool found = false;
            while (getline(file, line)) {
                size_t col_pos = line.find(':');
                if (col_pos == std::string::npos) {
                    continue;
                }
                if (commands[0].args[2] == line.substr(0, col_pos)) {
                    found = true;
                    raw_command = line.substr(col_pos + 1);
                    std::vector<command> new_cmd;
                    try {
                        new_cmd = parser(raw_command);
                        if (new_cmd.empty()) {
                            continue;
                        }
                    }
                    catch (const std::exception &e) {
                        std::cout << e.what() << std::endl;
                        continue;
                    }
                    return execute(new_cmd);
                }
            }
            if (found == false) {
                std::cout << "Command alias not found" << std::endl;
                return true;
            }
        }
        if (commands[0].args[1] == "list") {
            std::ifstream file(file_location);
            if (!file) {
                std::cout << "Can't open file" << std::endl;
                return true;
            }
            int i = 1;
            while (getline(file, line)) {
                std::cout << i << ". " << line << std::endl;
                i++;
            }
            return true;
        }
        if (commands[0].args[1] == "delete") {
            //TODO: switch to lambda
            std::ifstream infile(file_location);
            std::vector<std::string> temp_file_contents;
            if (!infile) {
                std::cout << "Can't open file" << std::endl;
                return true;
            }
            while (getline(infile, line)) {
                size_t col_pos = line.find(':');
                if (col_pos == std::string::npos || commands[0].args[2] == line.substr(0, col_pos)) {
                    continue;
                }
                temp_file_contents.push_back(line);
            }
            infile.close();

            std::ofstream outfile(file_location);
            for (const auto& l : temp_file_contents) {
                outfile << l << std::endl;
            }
            outfile.close();
            return true;
        }
    }


    //Multi commands handling
    auto in_fd = 0;
    int fd[2];
    std::vector<pid_t> pids;
    for (int i = 0; i < commands.size(); i++) {
        bool hasNext = i + 1 < commands.size();
        if (hasNext) {
            pipe(fd);
        }
        pid_t p_id = fork();

        // < 0 -> no child created
        if (p_id < 0) {
            std::cout << "Child not created" << std::endl;
        }
        // == 0 -> child process
        else if (p_id == 0) {
            //pipeline
            if (i != 0) {
                dup2(in_fd, 0);
                close(in_fd);
            }
            if (hasNext) {
                dup2(fd[1], 1);
            }

            // input/output
            int f_fd = -1;
            if (!commands[i].input_file.empty()) {
                f_fd = open(commands[i].input_file.c_str(), O_RDONLY);
                if (f_fd >= 0) {
                    dup2(f_fd, 0);
                    close(f_fd);
                } else {
                    perror("No input file found");
                    exit(1);
                }
            }
            if (!commands[i].output_file.empty()) {
                if (commands[i].output_mode == Output_mode::Append) {
                    f_fd = open(commands[i].output_file.c_str(), O_WRONLY | O_CREAT | O_APPEND, 0644);
                } else if (commands[i].output_mode == Output_mode::Overwrite) {
                    //0644 - permissions for file
                    f_fd = open(commands[i].output_file.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
                }
                if (f_fd >= 0) {
                    dup2(f_fd, 1);
                    close(f_fd);
                } else {
                    perror("Output file could not be created");
                    exit(1);
                }
            }

            //preparing arguments array
            std::vector<char *> argv;
            for (std::string &word: commands[i].args) {
                argv.push_back(const_cast<char *>(word.c_str()));
            }
            argv.push_back(nullptr);

            if (hasNext) {
                close(fd[1]);
                close(fd[0]);
            }

            //running command
            execvp(argv[0], argv.data());
            exit(1);
        }

        // > 0 -> parent process
        else if (p_id > 0) {
            if (in_fd != 0)
                close(in_fd);
            if (hasNext) {
                close(fd[1]);
                in_fd = fd[0];
            }

            pids.push_back(p_id);
        }
    }
    //waiting for all child processes to finish
    for (pid_t p_id: pids) {
        waitpid(p_id, nullptr, 0);
    }
    return true;
}
