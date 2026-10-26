//
// Created by kacper on 10/21/26.
//

#ifndef PIGEONCLI_DEBUG_H
#define PIGEONCLI_DEBUG_H

#include "debug.h"

#include <array>
#include <iostream>
#include <unistd.h>

#include "ansi_esc_helper.h"
#include "terminal.h"

void debug_show_var(int pos, auto var) {
    std::array<std::array<int,2>, 10> pos_array = {{
        {1,20},
        {2,20},
        {3,20},
        {4,20},
        {5,20},
        {6,20},
        {7,20},
        {8,20},
        {9,20},
        {10,20}
    }};
    move_cursor(cursor_action::SAVE);
    move_cursor(pos_array[pos][0], pos_array[pos][1]);
    std::cout<<"                "<<std::flush;
    std::cout<<var<<std::flush;
    move_cursor(cursor_action::LOAD);
}
void debug_breakpoint() {
    char byte;
    read(STDIN_FILENO, &byte, 1);
    while (byte != '\n' || byte != '\r') {
        debug_show_var(1,"STOP");
        read(STDIN_FILENO, &byte, 1);
    }
}
/*TODO:
 * - breakpoint
 * - fix show_var not cleaning old info
 */

#endif //PIGEONCLI_DEBUG_H
