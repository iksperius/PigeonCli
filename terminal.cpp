//
// Created by kacper on 10/14/26.
//

#include "terminal.h"
#include <unistd.h>
#include <iostream>

//constructor disabling echo and canonical mode
terminal_raw_mode::terminal_raw_mode() {
    //gets current console attributes
    tcgetattr(STDIN_FILENO, &orig_termios);
    struct termios raw = orig_termios;
    //zeroes bits for echo and icanon disabling them
    raw.c_lflag &= ~(ECHO | ICANON);
    //sets attributes from raw struct
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}
//deconstructor setting previous console attributes
terminal_raw_mode::~terminal_raw_mode() {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

std::string terminal_raw_mode::read_line() {
    std::string line;
    char c;
    while (true) {
        read(STDIN_FILENO, &c, 1);
        //enter sends either \n or \r
        if (c == '\n' || c == '\r') {
            std::cout << "\r\n" <<std::flush;
            return line;
        }
        //backspace sends either code 127 or \b
        if (c == '\b' || c == 127) {
            if (!line.empty()) {
                // moves back cursor, prints ' ' and moves back cursor again
                std::cout << "\b \b" << std::flush;
                line.pop_back();
            }
            continue;
        }
        std::cout << c << std::flush;
        line += c;
    }
}
