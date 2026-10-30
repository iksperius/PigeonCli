//
// Created by kacper on 10/26/26.
//

#ifndef PIGEONCLI_ANSI_ESC_HELPER_H
#define PIGEONCLI_ANSI_ESC_HELPER_H
#include <iostream>

namespace ansi {
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

    void clear_from_cur_pos();
    void move_cursor(direction direction);
    void move_cursor(size_t row,size_t col);
    void move_cursor(cursor_action action);
    void move_line(direction direction, size_t col);
    void beep();
}
#endif //PIGEONCLI_ANSI_ESC_HELPER_H
