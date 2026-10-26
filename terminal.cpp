//
// Created by kacper on 10/14/26.
//

#include "terminal.h"
#include "debug.h"

#include <unistd.h>
#include <iostream>
#include <sys/ioctl.h>

#include "ansi_esc_helper.h"
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
    //screen
    CSI,
    //terminal
    OSC,
    SS3
};
enum class arrow_key : char {
    UP = 'A',
    DOWN = 'B',
    RIGHT = 'C',
    LEFT = 'D'
};

static struct winsize ws;
static size_t cmd_len = 0;
static std::array<size_t, 2> cursor_pos{};

std::array<size_t ,2> get_cursor_pos() {
    return cursor_pos;
};
void set_cursor_pos(std::array<size_t, 2> pos) {
    cursor_pos = pos;
};


std::string terminal_raw_mode::read_line() {
    std::string line;
    char byte;
    sequence_states state = sequence_states::NONE;

    while (true) {
        //reading terminal window size
        ioctl(STDIN_FILENO, TIOCGWINSZ, &ws);
        //debug stuff
        // debug_show_var(1, cmd_len);
        // debug_show_var(2,line.length());
        // debug_show_var(3,ws.ws_col);
        // debug_show_var(4,ws.ws_row);
        // debug_show_var(5, get_cursor_pos()[0]);
        // debug_show_var(6, abs_cursor_pos[1]);

        //reading bytes from keyboard input
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
                if (cmd_len > 0) {
                    cmd_len--;
                    std::cout << "\b \b" << std::flush;
                    line.erase(cmd_len,1);
                    move_cursor(cursor_action::SAVE);
                    //clears line from current cursor position
                    ansi_clear_from_cur_pos();
                    for (size_t i = cmd_len; i < line.length(); i++) {
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
                    cmd_len++;
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
                    if (cmd_len < line.length()) {
                        // if (get_cursor_pos()[1] >= ws.ws_col) {
                        //     //"\x1b[S" - scrolls down
                        //     std::cout<< "\x1b[E" << std::flush;
                        // }
                        cmd_len++;
                        ansi_move_cursor(RIGHT);
                    }
                    else {
                        //beep
                        std::cout<<"\07" <<std::flush;
                    }
                    break;
                case arrow_key::LEFT:
                    if (cmd_len > 0) {
                        cmd_len--;
                        ansi_move_cursor(LEFT);
                    }
                    else {
                        // if (get_cursor_pos()[1] <= 0) {
                        //     std::cout<< "\x1b[F" << std::flush;
                        // }
                        // else {
                        //     //beep
                        //     std::cout<<"\07" <<std::flush;
                        // }
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
        /* - moving to another line is really scuffed
         */
        line.insert(cmd_len, std::string(1, byte));
        if (cmd_len+1 < line.length()) {
            std::cout<<line[cmd_len]<<std::flush;
            cmd_len++;
            move_cursor(cursor_action::SAVE);
            for (size_t i = cmd_len; i < line.length(); i++) {
                std::cout << line[i] <<std::flush;
            }
            move_cursor(cursor_action::LOAD);
        }
        else {
            std::cout<<byte<<std::flush;
            cmd_len++;
        }
    }
}
