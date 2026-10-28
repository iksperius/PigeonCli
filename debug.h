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
namespace debug {
    void show_var(int pos, auto var) {
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
        ansi::move_cursor(ansi::cursor_action::SAVE);
        ansi::move_cursor(pos_array[pos][0], pos_array[pos][1]);
        std::cout<<"                "<<std::flush;
        std::cout<<var<<std::flush;
        ansi::move_cursor(ansi::cursor_action::LOAD);
    }
    /*TODO:
     * - breakpoint
     */
    void breakpoint() {
        char byte;
        read(STDIN_FILENO, &byte, 1);
        while (byte != '\n' || byte != '\r') {
            show_var(1,"STOP");
            read(STDIN_FILENO, &byte, 1);
        }
    }


}


#endif //PIGEONCLI_DEBUG_H
