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
enum sequence_states {
    NONE,
    SEQUENCE,
    CSI,
    OSC,
    SS3
};
enum class arrow_key : char {
    UP = 'A',
    DOWN = 'B',
    RIGHT = 'C',
    LEFT = 'D'
};

std::string terminal_raw_mode::read_line() {
    std::string line;
    char byte;
    sequence_states state = sequence_states::NONE;

    size_t cursor_pos = 0;
    while (true) {
        read(STDIN_FILENO, &byte, 1);
        //enter sends either \n or \r
        if (byte == '\n' || byte == '\r') {
            std::cout << "\r\n" <<std::flush;
            return line;
        }
        //backspace sends either code 127 or \b
        if (byte == '\b' || byte == 127) {
            if (!line.empty()) {
                // moves back cursor, prints ' ' and moves back cursor again
                if (cursor_pos > 0) {
                    cursor_pos--;
                    std::cout << "\b \b" << std::flush;
                    line.erase(cursor_pos);
                }
            }
            continue;
        }
        if (byte == '\033') {
            state = sequence_states::SEQUENCE;
            continue;
        }
        if (state == sequence_states::SEQUENCE) {
            switch (byte) {
                case '[':
                    state = sequence_states::CSI;
                    break;
                case ']':
                    state = sequence_states::OSC;
                    break;
                case 'O':
                    state = sequence_states::SS3;
                    break;
                default:
                    state = sequence_states::NONE;
                    std::cout << byte <<std::flush;
                    cursor_pos++;
                    break;
            }
            continue;
        }
        if (state == sequence_states::CSI) {
            state = sequence_states::NONE;

            switch (static_cast<arrow_key>(byte)) {
                case arrow_key::UP:
                    break;
                case arrow_key::DOWN:
                    break;
                case arrow_key::RIGHT:
                    if (cursor_pos < line.length()) {
                        cursor_pos++;
                        std::cout << "\033[C" << std::flush;
                    }
                    break;
                case arrow_key::LEFT:
                    if (cursor_pos > 0) {
                        cursor_pos--;
                        std::cout << "\033[D" << std::flush;
                    }
                    break;
            }
            continue;
        }
        if (state == sequence_states::OSC) {
            state = sequence_states::NONE;
            continue;
        }
        if (state == sequence_states::SS3) {
            state = sequence_states::NONE;
            continue;
        }
        cursor_pos++;
        std::cout << byte << std::flush;
        line += byte;
    }
}
