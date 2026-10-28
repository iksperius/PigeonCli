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
struct CursorPos {
    size_t x;
    size_t y;
};
CursorPos get_cursor_pos();
void set_cursor_pos(CursorPos pos);


#endif //PIGEONCLI_TERMINAL_H
