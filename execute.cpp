//
// Created by kacper on 10/14/26.
//

#include <fcntl.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <unistd.h>
#include <unordered_map>
#include <sys/wait.h>

#include "pigeon.h"
#include "execute.h"

#include <cstring>


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
        return handle_pigeon(commands);
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
            close(in_fd);
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
            if (execvp(argv[0], argv.data()) == -1) {
                std::cout << "Unknown command: " << argv[0] << std::endl;
                // std::cout << std::strerror(errno) << std::endl;
            }
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
