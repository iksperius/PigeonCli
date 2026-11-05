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
    std::cout<<"\033[?7l"<<std::flush;
    std::cout<<"\033]0;PigeonCLI\07"<<std::flush;
}
//deconstructor setting previous console attributes
terminal_raw_mode::~terminal_raw_mode() {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
    std::cout<<"\033[?7h"<<std::flush;
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

static winsize ws;
static size_t cmd_pos = 0;

// static std::array<size_t, 2> cursor_pos{};
static CursorPos cursor_pos = CursorPos();

CursorPos get_cursor_pos() {
    return cursor_pos;
};
void set_cursor_pos(const CursorPos pos) {
    cursor_pos.x = pos.x;
    cursor_pos.y = pos.y;
};

winsize get_ws() {
    return ws;
}
/*TODO
 *  - scroll when not enough space on bottom of the terminal
 */
std::string terminal_raw_mode::read_line() {
    std::string line;
    char byte;
    sequence_states state = sequence_states::NONE;

    while (true) {
        //reading terminal window size
        ioctl(STDIN_FILENO, TIOCGWINSZ, &ws);
        //debug stuff
        debug::show_var(2, line.length(), "linelength");
        debug::show_var(3, cmd_pos, "cmdpos");
        debug::show_var(4, get_cursor_pos().x, "cursorPos");
        debug::show_var(5, ws.ws_col, "wscol");

        //reading bytes from keyboard input
        read(STDIN_FILENO, &byte, 1);

        //enter sends either \n or \r
        if (byte == '\n' || byte == '\r') {
            std::cout << "\r\n" <<std::flush;
            cmd_pos = 0;
            cursor_pos.x = 1;
            return line;
        }

        //backspace sends either code 127 or \b
        if (byte == '\b' || byte == 127) {
            if (cmd_pos != 0) {
                if (cursor_pos.x == 0) {
                    ansi::move_line(ansi::UP);
                    cursor_pos.x = ws.ws_col;
                    std::cout << " " << std::flush;
                }
                else {
                    // moves back cursor, prints ' ' and moves back cursor again
                    std::cout << "\b \b" << std::flush;
                }
                cmd_pos--;
                cursor_pos.x--;
                line.erase(cmd_pos,1);
                ansi::clear_from_cur_pos();
                ansi::move_cursor(ansi::cursor_action::SAVE);
                for (size_t i = cmd_pos; i < line.length(); i++) {
                    if (i == ws.ws_col) {
                        ansi::move_line(ansi::DOWN);
                    }
                    std::cout << line[i] <<std::flush;
                }
                ansi::move_cursor(ansi::cursor_action::LOAD);
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
                case arrow_key::RIGHT: {
                    if (cmd_pos < line.length()) {
                        ansi::move_cursor(ansi::RIGHT);
                        if (get_cursor_pos().x == ws.ws_col) {
                            cmd_pos++;
                            cursor_pos.x = 0;
                            ansi::move_line(ansi::DOWN);
                            break;
                        }
                        cmd_pos++;
                        break;
                    }
                    ansi::beep();
                }
                break;
                case arrow_key::LEFT:
                if (cmd_pos > 0) {
                    if (get_cursor_pos().x == 0) {
                        cmd_pos--;
                        //-1 cuz we dont want cursor outside of the screen
                        cursor_pos.x = ws.ws_col-1;
                        ansi::move_line(ansi::UP);
                        break;
                    }
                    cmd_pos--;
                    ansi::move_cursor(ansi::LEFT);
                    break;
                }
                ansi::beep();
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

        //insert typed letters
        line.insert(cmd_pos, std::string(1, byte));
        //insert letters mid line
        if (cmd_pos+1 < line.length()) {

            std::cout<<line[cmd_pos]<<std::flush;
            ansi::move_cursor(ansi::SAVE);
            for (size_t i = cmd_pos+1; i < line.length(); i++) {
                if (i == ws.ws_col) {
                    ansi::move_line(ansi::DOWN);
                }
                std::cout << line[i] <<std::flush;
            }
            ansi::move_cursor(ansi::LOAD);
            cursor_pos.x++;
            cmd_pos++;
        }
        else {
            //append letters at the end of line
            if (cursor_pos.x == ws.ws_col) {
                cursor_pos.x = 0;
                ansi::move_line(ansi::DOWN);
                std::cout<<byte<<std::flush;
            }
            else {
                std::cout<<byte<<std::flush;
            }
            cmd_pos++;
            cursor_pos.x++;
        }
    }
}
