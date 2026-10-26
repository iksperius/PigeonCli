//
// Created by kacper on 10/26/26.
//

#ifndef PIGEONCLI_ANSI_ESC_HELPER_H
#define PIGEONCLI_ANSI_ESC_HELPER_H
#include <iostream>

enum direction {
    UP,
    DOWN,
    RIGHT,
    LEFT,
};
enum cursor_action{
    SAVE,
    LOAD
};

void ansi_clear_from_cur_pos();
void ansi_move_cursor(direction direction);
void move_cursor(size_t row,size_t col);
void move_cursor(cursor_action action);
//column up/down
void move_cursor(direction direction);

#endif //PIGEONCLI_ANSI_ESC_HELPER_H
