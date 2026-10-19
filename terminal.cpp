//
// Created by kacper on 10/14/26.
//

#include "terminal.h"
#include <unistd.h>
#include <iostream>
#include <array>
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
        showvar(1, cursor_pos);
        showvar(2,line.length());
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
                    line.erase(cursor_pos,1);
                    move_cursor(cursor_action::SAVE);
                    //clears line from current cursor position
                    std::cout<<"\033[0K"<<std::flush;
                    for (size_t i = cursor_pos; i < line.length(); i++) {
                        std::cout << line[i] <<std::flush;
                    }
                    move_cursor(cursor_action::LOAD);
                }
            }
            continue;
        }
        if (byte == '\033') {
            state = sequence_states::SEQUENCE;
            continue;
        }
        //state handling
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
        //arrow keys
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
        //TODO:
        /*
         * - deleting doesnt move characters (check why removing characters in middle of word makes rest of command not run)
         * - moving to another line is really scuffed
         * - make it somehow possible to check each variable value - external window/command/step-by-step
         */
        line.insert(cursor_pos, std::string(1, byte));
        if (cursor_pos+1 < line.length()) {
            std::cout<<line[cursor_pos]<<std::flush;
            cursor_pos++;
            move_cursor(cursor_action::SAVE);
            for (size_t i = cursor_pos; i < line.length(); i++) {
                std::cout << line[i] <<std::flush;
            }
            move_cursor(cursor_action::LOAD);
        }
        else {
            std::cout<<byte<<std::flush;
            cursor_pos++;
        }
    }
}

void showvar(int pos, auto var) {
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
    std::cout<<var;
    move_cursor(cursor_action::LOAD);
}

void move_cursor(int row,int col) {
    std::cout<<"\x1b[";
    std::cout<<row;
    std::cout<<";";
    std::cout<<col;
    std::cout<<"H"<<std::flush;
};
void move_cursor(cursor_action action) {
    if (action == cursor_action::SAVE) {
        std::cout<<"\x1b[s"<<std::flush;
        return;
    }
    if (action == cursor_action::LOAD) {
        std::cout<<"\x1b[u"<<std::flush;
        return;
    }
}

