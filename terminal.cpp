//
// Created by kacper on 10/14/26.
//

#include "terminal.h"
#include "debug.h"

#include <unistd.h>
#include <iostream>
#include <sys/ioctl.h>
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

struct winsize ws;

std::string terminal_raw_mode::read_line() {
    std::string line;
    char byte;
    sequence_states state = sequence_states::NONE;
    //start at 1 cuz >
    size_t cursor_pos = 0;


    while (true) {
        std::array<int, 2> abs_cursor_pos = get_cursor_pos();
        //reading terminal window size
        ioctl(STDIN_FILENO, TIOCGWINSZ, &ws);
        //debug stuff
        // debug_show_var(1, cursor_pos);
        // debug_show_var(2,line.length());
        // debug_show_var(3,ws.ws_col);
        // debug_show_var(4,ws.ws_row);
        // debug_show_var(5, get_cursor_pos()[0]);
        debug_show_var(6, abs_cursor_pos[1]);

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
                        // if (get_cursor_pos()[1] >= ws.ws_col) {
                        //     //"\x1b[S" - scrolls down
                        //     std::cout<< "\x1b[E" << std::flush;
                        // }
                        cursor_pos++;
                        std::cout << "\033[C" << std::flush;
                    }
                    else {
                        //beep
                        std::cout<<"\07" <<std::flush;
                    }
                    break;
                case arrow_key::LEFT:
                    if (cursor_pos > 0) {
                        cursor_pos--;
                        std::cout << "\033[D" << std::flush;
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
/*TODO
 * - fix this piece of shit
 *      - gets random stuff from keyboard mixed in
 *      - fix misleading names of cursor_pos and get_cursor_pos
 *      - save get_cursor_pos to variable instead of rerunning
 *      - add comments
 *      - nvm it doesnt work at all
 */
std::array<int ,2> get_cursor_pos() {
    std::array<int,2> cursor_pos = {};
    std::cout<<"\033[6n"<<std::flush;
    u_char byte;
    char raw_cords[32];
    read(STDIN_FILENO, &byte, 1);
    int i = 0;
    while (byte != 'R') {
        debug_breakpoint();
        debug_show_var(4,byte);
        if (i > 31) {
            break;
        }
        raw_cords[i] = byte;
        i++;
        read(STDIN_FILENO, &byte, 1);
    }
    raw_cords[i] = '\0';
    sscanf(raw_cords, "\033[%d;%d",&cursor_pos[0], &cursor_pos[1]);;
    return cursor_pos;
}

