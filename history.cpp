//
// Created by kacper on 11/10/26.
//

#include "history.h"

#include <deque>
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
        ansi::move_cursor(get_cursor_pos().y, 0);
        history_list.clear();
        std::string line;
        if (infile.is_open()) {
            while (std::getline(infile, line)) {
                history_list.push_front(line);
            }
        }
        size_t cmd_pos = get_cmd_pos();
        size_t prompt_len = 0;
        winsize ws = get_ws();
        for (size_t i = 0; i < line.length(); i++) {
            if (i != cmd_pos && (prompt_len + i) % ws.ws_col == 0) {
                ansi::move_line(ansi::DOWN);
            }
            prompt_len += 1;
            std::cout << line[i];
        }
    };
    void reset() {
        index = 0;
        history_list.clear();
    };
    void next() {
        index++;
        history_list.pop_front();
        display();
    };
    void previous() {
        index--;
        display();
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
    void save_file() {
        infile.close();
        outfile.close();
    }
    void save(const std::string& line){
        history_list.pop_front();
        history_list.push_front(line);
    }
}
