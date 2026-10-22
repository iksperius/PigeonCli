//
// Created by kacper on 10/14/26.
//


#ifndef PIGEONCLI_TERMINAL_H
#define PIGEONCLI_TERMINAL_H


#include <string>
#include <termios.h>

enum cursor_action{
    SAVE,
    LOAD
};

class terminal_raw_mode {
private:
    struct termios orig_termios{};
public:
    terminal_raw_mode();
    ~terminal_raw_mode();
    std::string read_line();
};
void move_cursor(int row,int col);
void move_cursor(cursor_action action);
std::array<int ,2> get_cursor_pos();

#endif //PIGEONCLI_TERMINAL_H
