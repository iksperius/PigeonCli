#include <iostream>
#include <sstream>
#include <unistd.h>
#include <string>
#include <vector>
#include <cstdlib>
#include <sys/wait.h>

int main() {

    std::string line;

    while (true) {
        //Parser
        std::cout << ">";
        std::getline(std::cin, line);
        std::vector<std::vector<std::string>> commands;
        std::vector<std::string> temp_command;
        std::string temp_arg;
        bool in_quote = false;
        for (char c : line) {
            if (c == '"') {
                in_quote = !in_quote;
                continue;
            }

            //quote handling
            if (in_quote) {
                temp_arg += c;
                continue;
            }

            if (c == ' ') {
                if (!temp_arg.empty()) {
                    temp_command.push_back(temp_arg);
                    temp_arg.clear();
                }
                continue;
            }
            if (c == '|') {
                if (!temp_arg.empty()) {
                    temp_command.push_back(temp_arg);
                }
                if (!temp_command.empty()) {
                    commands.push_back(temp_command);
                }
                temp_command.clear();
                temp_arg.clear();
                continue;
            }

            temp_arg += c;
        }
        if (in_quote) {
            std::cout<<"Quote not closed" << std::endl;
            continue;
        }
        if (!temp_arg.empty()) {
            temp_command.push_back(temp_arg);
        }
        if (!temp_command.empty()) {
            commands.push_back(temp_command);
        }
        if (commands.empty()) {
            continue;
        }



        if (commands[0][0] == "exit") {
            break;
        }
        if (commands[0][0] == "cd") {
            char* home = getenv("HOME");
            if (commands[0].size() > 1) {
                if (commands[0][1][0] == '~' && home != nullptr) {
                    commands[0][1] = getenv("HOME") + commands[0][1].substr(1);
                }
                if (chdir(commands[0][1].c_str()) == -1) {
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
                if (i != 0) {
                    dup2( in_fd, 0);
                    close(in_fd);
                }
                if (hasNext) {
                    dup2( fd[1], 1);
                }

                //preparing arguments array
                std::vector<char*> argv;
                for (std::string& word : commands[i]) {
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