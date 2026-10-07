#include <iostream>
#include <sstream>
#include <unistd.h>
#include <string>
#include <vector>
#include <cstdlib>
#include <format>
#include <sys/wait.h>
#include <fcntl.h>

int main() {

    std::string line;

    enum class Redirect_state {
        None,
        Input,
        Output,
        Append
    };

    enum class Output_mode{
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


    while (true) {
        //Parser
        std::cout << ">";
        std::getline(std::cin, line);
        std::vector<command> commands;
        command temp_command;
        std::string temp_arg;
        bool in_quote = false;
        bool syntax_error = false;
        Redirect_state redir_state = Redirect_state::None;

        //Lambda function for pushing arguments, input/output file names
        auto arg_push = [&]() {
            if (redir_state == Redirect_state::Input) {
                temp_command.input_file = temp_arg;
            }
            else if (redir_state == Redirect_state::Output) {
                temp_command.output_file = temp_arg;
                temp_command.output_mode = Output_mode::Overwrite;
            }
            else if (redir_state == Redirect_state::Append) {
                temp_command.output_file = temp_arg;
                temp_command.output_mode = Output_mode::Append;
            }
            else {
                temp_command.args.push_back(temp_arg);
            }
            temp_arg.clear();
            redir_state = Redirect_state::None;
        };

        for (int i = 0; i < line.size(); i++) {
            bool hasNext = i + 1 < line.size();

            //quote handling
            if (line[i] == '"') {
                in_quote = !in_quote;
                continue;
            }

            //text in quote handling
            if (in_quote) {
                temp_arg += line[i];
                continue;
            }

            if (line[i] == '<') {
                if (!temp_arg.empty()) {
                    arg_push();
                }
                redir_state = Redirect_state::Input;
                continue;
            }
            if (line[i] == '>') {
                if (!temp_arg.empty()) {
                    arg_push();
                }
                if (hasNext  && line[i+1] == '>') {
                    redir_state = Redirect_state::Append;
                    i++;
                }
                else {
                    redir_state = Redirect_state::Output;
                }
                continue;
            }


            if (line[i] == ' ') {
                if (!temp_arg.empty()) {
                    arg_push();
                }
                continue;
            }
            if (line[i] == '|') {
                if (!temp_arg.empty()) {
                    arg_push();
                }
                if (redir_state != Redirect_state::None) {
                    std::cout<<"Syntax error near unexpected token '|" <<std::endl;
                    syntax_error = true;
                    break;
                }
                if (!temp_command.args.empty()) {
                    commands.push_back(temp_command);
                }
                temp_command = command{};
                temp_arg.clear();
                continue;
            }
            //home handling
            if (line[i] == '~' && !in_quote && temp_arg.empty()) {
                if (i + 1 == line.size() || line[i + 1] == '/' || line[i + 1] == ' ' || line[i + 1] == '|') {
                    char* home = getenv("HOME");
                    if (home != nullptr) {
                        temp_arg += home;
                        continue;
                    }
                }
            }

            temp_arg += line[i];
        }
        if (in_quote) {
            std::cout<<"Quote not closed" << std::endl;
            continue;
        }
        if (syntax_error) {
            continue;
        }
        if (!temp_arg.empty()) {
            arg_push();
        }
        if (redir_state != Redirect_state::None) {
            std::cout << "Expected a string, but found end of the input" <<std::endl;
            continue;
        }

        if (!temp_command.args.empty()) {
            commands.push_back(temp_command);
        }
        if (commands.empty()) {
            continue;
        }



        if (commands[0].args[0] == "exit") {
            break;
        }
        if (commands[0].args[0] == "cd") {
            char* home = getenv("HOME");
            if (commands[0].args.size() > 1) {
                if (chdir(commands[0].args[1].c_str()) == -1) {
                    std::cout<<"Not a valid directory"<<std::endl;
                }
                continue;
            }
            if (home != nullptr) chdir(getenv("HOME"));
            else {
                std::cout<<"Home is not set"<<std::endl;
            }
            continue;
        }
        //TODO: Add funny pigeon command
        // if (commands[0].args[0] == "pigon") {
        //     std::cout<<"Pigon"<<std::endl;
        //     continue;
        // }

        //TODO: add clipboard for commands, sc list, sc add etc


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
                std::cout<<"Child not created" << std::endl;
            }
            // == 0 -> child process
            else if (p_id == 0) {
                //pipeline
                if (i != 0) {
                    dup2( in_fd, 0);
                    close(in_fd);
                }
                if (hasNext) {
                    dup2( fd[1], 1);
                }

                // input/output
                int f_fd = -1;
                if (!commands[i].input_file.empty()) {
                    f_fd = open(commands[i].input_file.c_str(), O_RDONLY);
                    if (f_fd >= 0) {
                        dup2(f_fd, 0);
                        close(f_fd);
                    }
                    else {
                        perror("No input file found");
                        exit(1);
                    }
                }
                if (!commands[i].output_file.empty()) {
                    if (commands[i].output_mode == Output_mode::Append) {
                        f_fd = open(commands[i].output_file.c_str(), O_WRONLY | O_CREAT | O_APPEND, 0644);
                    }
                    else if (commands[i].output_mode == Output_mode::Overwrite) { //0644 - permissions for file
                        f_fd = open(commands[i].output_file.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
                    }
                    if (f_fd >= 0) {
                        dup2(f_fd, 1);
                        close(f_fd);
                    }
                    else {
                        perror("Output file could not be created");
                        exit(1);
                    }

                }

                //preparing arguments array
                std::vector<char*> argv;
                for (std::string& word : commands[i].args) {
                        argv.push_back((char*)word.c_str());
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
        for (pid_t p_id : pids) {
            waitpid(p_id, nullptr, 0);
        }
    }

    return 0;
}