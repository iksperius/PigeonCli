//
// Created by kacper on 10/14/26.
//


#ifndef PIGEONCLI_TERMINAL_H
#define PIGEONCLI_TERMINAL_H


#include <array>
#include <string>
#include <termios.h>

class terminal_raw_mode {
private:
    struct termios orig_termios{};
public:
    terminal_raw_mode();
    ~terminal_raw_mode();
    std::string read_line();
};
//row/col
std::array<size_t ,2> get_cursor_pos();
void set_cursor_pos(std::array<size_t, 2> pos);


#endif //PIGEONCLI_TERMINAL_H
