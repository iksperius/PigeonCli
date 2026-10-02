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
        std::cout << ">";
        std::getline(std::cin, line);
        std::string formated_line = "";
        for (char c : line) {
            if (c == '|') {
                formated_line += " | ";
            }
            else {
                formated_line += c;
            }
        }
        std::stringstream ss(formated_line);

        std::string sep_word;
        std::vector<std::string> command;
        while (ss >> sep_word) {
            command.push_back(sep_word);
        }
        if (command.empty()) {
            continue;
        }
        if (command[0] == "exit") {
            break;
        }
        if (command[0] == "cd") {
            char* home = getenv("HOME");
            if (command.size() > 1) {
                if (command[1][0] == '~' && home != nullptr) {
                    command[1] = getenv("HOME") + command[1].substr(1);
                }
                if (chdir(command[1].c_str()) == -1) {
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

        std::vector<std::vector<std::string>> commands;
        std::vector<std::string> current_command;
        for (std::string& s : command) {
            if (s == "|" && !current_command.empty()) {
                commands.push_back(current_command);
                current_command.clear();
            }
            else {
                current_command.push_back(s);
            }
        }
        if (!current_command.empty()) {
            commands.push_back(current_command);
        }

        auto in_fd = 0;
        int fd[2];
        std::vector<pid_t> pids;
        for (int i = 0; i < commands.size(); i++) {
            bool hasNext = i + 1 < commands.size();
            if (hasNext) {
                pipe(fd);
            }
            pid_t p_id = fork();

            if (p_id < 0) {
                std::cout<<"Child not created" << std::endl;
            }
            else if (p_id == 0) {
                if (i != 0) {
                    dup2( in_fd, 0);
                    close(in_fd);
                }
                if (hasNext) {
                    dup2( fd[1], 1);
                }

                std::vector<char*> argv;
                for (std::string& word : commands[i]) {
                        argv.push_back((char*)word.c_str());
                }
                argv.push_back(nullptr);
                if (hasNext) {
                    close(fd[1]);
                    close(fd[0]);
                }
                execvp(argv[0], argv.data());
                exit(1);
            }
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
        for (pid_t p_id : pids) {
            waitpid(p_id, nullptr, 0);
        }
    }

    return 0;
}