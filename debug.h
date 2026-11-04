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
        ansi::move_cursor(ansi::cursor_action::SAVE);
        ansi::move_cursor(pos, 20);
        std::cout<<"    "<<std::flush;
        ansi::move_cursor(pos, 20);
        std::cout<<var<<std::flush;
        ansi::move_cursor(ansi::cursor_action::LOAD);
    }
    void show_var(int pos, auto var, std::string desc) {
        ansi::move_cursor(ansi::cursor_action::SAVE);
        ansi::move_cursor(pos, 20);
        std::cout<<"    "<<std::flush;
        std::cout<<desc<<std::flush;
        ansi::move_cursor(pos, 20);
        std::cout<<var<<std::flush;
        ansi::move_cursor(ansi::cursor_action::LOAD);
    }
}


#endif //PIGEONCLI_DEBUG_H
