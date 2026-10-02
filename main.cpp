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
        std::vector<std::string> line_separated;
        std::stringstream ss(line);

        std::string sep_word;
        while (ss >> sep_word) {
            line_separated.push_back(sep_word);
        }
        if (line_separated.empty()) {
            continue;
        }
        if (line_separated[0] == "exit") {
            break;
        }
        if (line_separated[0] == "cd") {
            char* home = getenv("HOME");
            if (line_separated.size() > 1) {
                if (line_separated[1][0] == '~' && home != nullptr) {
                    line_separated[1] = getenv("HOME") + line_separated[1].substr(1);
                }
                if (chdir(line_separated[1].c_str()) == -1) {
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

        pid_t p_id = fork();
        if (p_id < 0) {
            std::cout<<"Child not created" <<std::endl;
        }
        else if (p_id == 0) {
            std::vector<char*> argv;
            for (std::string& word : line_separated) {
                argv.push_back((char*)word.c_str());
            }
            argv.push_back(nullptr);
            execvp(argv[0], argv.data());
            exit(1);
        }
        else {
            waitpid(p_id, nullptr, 0);
        }
    }

    return 0;
}