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
    static std::string read_line();

};
//we could change this to instead tracking only cmd_len -> length of whole command and then modulo is by ws.col, this way we dont have to track 2 mostly same variables independently
struct CursorPos {
    size_t x = 1;
    //TODO: unused, can change to CursorPosX as we wont track y anyway

    size_t y = 0;
};
CursorPos get_cursor_pos();
void set_cursor_pos(CursorPos pos);


#endif //PIGEONCLI_TERMINAL_H
