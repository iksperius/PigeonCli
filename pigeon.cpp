//
// Created by kacper on 10/14/26.
//
#include <filesystem>
#include <fstream>
#include <iostream>
#include <unordered_map>

#include "pigeon.h"
#include "execute.h"

bool handle_pigeon(std::vector<command> commands) {
    std::unordered_map<std::string, std::string> pigeon_commands;

        //loading file to map
        char *home = getenv("HOME");
        if (home == nullptr) {
            std::cout << "Can't open file" << std::endl;
            return true;
        }
        std::string file_location = static_cast<std::string>(home) + "/.config/PigeonCli/saved_commands";
        std::string temp_file_location = static_cast<std::string>(home) + "/.config/PigeonCli/saved_commands.tmp";
        //trim saved_commands file from path
        std::filesystem::create_directories(std::filesystem::path(file_location).parent_path());

        //filling pigeon_commands map with commands from file
        std::ifstream infile(file_location);
        if (infile.is_open()) {
            std::string line;
            while (getline(infile, line)) {
                //colon is used to separate name and command
                size_t col_pos = line.find(':');
                if (col_pos == std::string::npos) {
                    continue;
                }
                pigeon_commands[line.substr(0,col_pos)] = line.substr(col_pos+1);
            }
        }
        infile.close();

        //writes pigeon_commands map to file
        auto sync_file = [&]() {
            std::ofstream temp_outfile(temp_file_location, std::ios::trunc);
            for (auto& [name, command] : pigeon_commands) {
                temp_outfile << name << ':' << command << std::endl;
            }
            temp_outfile.close();
            std::filesystem::rename(temp_file_location, file_location);
        };

        if (commands[0].args.size() < 2) {
            std::cout << "Usage: pigeon <save|run|list|delete> ..." << std::endl;
            return true;
        }


        if (commands[0].args[1] == "save") {
            if (commands[0].args.size() < 4) {
                std::cout << "Usage: pigeon save <name> \"command\" " << std::endl;
                return true;
            }
            std::string temp_command;
            for (int i = 3; i < commands[0].args.size(); i++) {
                temp_command += commands[0].args[i] + ' ';
            }
            pigeon_commands[commands[0].args[2]] = temp_command;

            sync_file();
            return true;
        }

        if (commands[0].args[1] == "run") {
            if (commands[0].args.size() < 3) {
                std::cout << "Usage: pigeon run <name> \"command\" " << std::endl;
                return true;
            }
            if (!pigeon_commands.contains(commands[0].args[2])) {
                std::cout << "Command alias not found" << std::endl;
                return true;
            }
            std::vector<command> new_cmd;
            try {
                new_cmd = parser(pigeon_commands[commands[0].args[2]]);
                if (new_cmd.empty()) {
                    return true;
                }
            }
            catch (std::runtime_error &e) {
                std::cout << e.what() << std::endl;
                return true;
            }
            return execute(new_cmd);
        }

        if (commands[0].args[1] == "list") {
            for (const auto& [name, command] : pigeon_commands) {
                std::cout << name << ":" <<command << std::endl;
            }
            return true;
        }

        if (commands[0].args[1] == "delete") {
            if (commands[0].args.size() < 3) {
                std::cout << "Usage: pigeon delete [name]" << std::endl;
                return true;
            }
            if (!pigeon_commands.contains(commands[0].args[2])) {
                std::cout << "Command alias not found" << std::endl;
                return true;
            }
            pigeon_commands.erase(commands[0].args[2]);
            sync_file();
            return true;
        }
    return false;
};