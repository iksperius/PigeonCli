//
// Created by kacper on 11/10/26.
//

#include "history.h"

#include <deque>
#include <filesystem>
#include <fstream>

#include "ansi_esc_helper.h"
#include "terminal.h"

static unsigned int index = 0;
std::string file_location;
std::string temp_file_location;
std::ofstream outfile;
std::ifstream infile;
static std::deque<std::string> history_list;
namespace history {
    void display() {
        ansi::move_cursor(ansi::SAVE);
        ansi::beginning_of_line();
        std::cout<<">"<<std::flush;
        // history_list.clear();
        // std::string line;
        // if (infile.is_open()) {
        //     while (std::getline(infile, line)) {
        //         history_list.push_front(line);
        //     }
        // }
        size_t cmd_pos = get_cmd_pos();
        size_t prompt_len = 0;
        winsize ws = get_ws();
        for (size_t i = 0; i < history_list[index].length(); i++) {
            if (i != cmd_pos && (prompt_len + i) % ws.ws_col == 0) {
                ansi::move_line(ansi::DOWN);
            }
            prompt_len += 1;
            std::cout << history_list[index][i];
        }
        ansi::move_cursor(ansi::LOAD);
    };
    void reset() {
        index = 0;
        history_list.clear();
    };
    void next() {
        if (index < history_list.size()) {
            display();
            index++;
        }
        // history_list.pop_front();
    };
    void previous() {
        if (index > 1) {
            display();
            index--;
        }
    };
    void open_read() {
        //loading file to map
        char *home = getenv("HOME");
        if (home == nullptr) {
            std::cout << "Can't open file" << std::endl;
        }
        temp_file_location = static_cast<std::string>(home) + "/.config/PigeonCli/saved_history.tmp";
        infile.open(temp_file_location.c_str());
    }
    void close() {
        infile.close();
    }
    void sync_file() {
    }
    void save(const std::string& line){
        // history_list.pop_front();
        char *home = getenv("HOME");
        if (home == nullptr) {
            std::cout << "Can't open file" << std::endl;
        }
        std::string file_location = static_cast<std::string>(home) + "/.config/PigeonCli/saved_history";
        temp_file_location = static_cast<std::string>(home) + "/.config/PigeonCli/saved_history.tmp";
        std::filesystem::create_directories(std::filesystem::path(file_location).parent_path());

        history_list.clear();
        std::ifstream infile(file_location);
        if (infile.is_open()) {
            std::string ln;
            while (getline(infile, ln)) {
                history_list.push_back(ln);
            }
        }
        history_list.push_back(line);
        infile.close();

        std::ofstream temp_outfile(temp_file_location, std::ios::trunc);
        for (const auto& cmd : history_list) {
            temp_outfile << cmd << std::endl;
        }
        temp_outfile.close();
        std::filesystem::rename(temp_file_location, file_location);
    }

    //initial sync file -> memory
    void save(){
        // history_list.pop_front();
        char *home = getenv("HOME");
        if (home == nullptr) {
            std::cout << "Can't open file" << std::endl;
        }
        std::string file_location = static_cast<std::string>(home) + "/.config/PigeonCli/saved_history";
        temp_file_location = static_cast<std::string>(home) + "/.config/PigeonCli/saved_history.tmp";
        std::filesystem::create_directories(std::filesystem::path(file_location).parent_path());

        history_list.clear();
        std::ifstream infile(file_location);
        if (infile.is_open()) {
            std::string ln;
            while (getline(infile, ln)) {
                history_list.push_back(ln);
            }
        }
        infile.close();

        std::ofstream temp_outfile(temp_file_location, std::ios::trunc);
        for (const auto& cmd : history_list) {
            temp_outfile << cmd << std::endl;
        }
        temp_outfile.close();
        std::filesystem::rename(temp_file_location, file_location);
    }
}
