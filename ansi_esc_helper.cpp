//
// Created by kacper on 10/26/26.
//
#include "ansi_esc_helper.h"

#include <iostream>
#include <ostream>
#include "terminal.h"
void ansi_clear_from_cur_pos() {
    std::cout<<"\033[0K"<<std::flush;
}

void ansi_move_cursor(const direction direction) {
    switch (direction) {
        case UP:
            std::cout<<"\033[A"<<std::flush;
            set_cursor_pos({get_cursor_pos()[0], get_cursor_pos()[1]  + 1});
            break;
        case DOWN:
            std::cout<<"\033[B"<<std::flush;
            set_cursor_pos({get_cursor_pos()[0] + 1, get_cursor_pos()[1] - 1});
            break;
        case RIGHT:
            std::cout<<"\033[C"<<std::flush;
            set_cursor_pos({get_cursor_pos()[0] + 1, get_cursor_pos()[1]});
            break;
        case LEFT:
            std::cout<<"\033[D"<<std::flush;
            set_cursor_pos({get_cursor_pos()[0] - 1, get_cursor_pos()[1]});
            break;
    }
}
void move_cursor(size_t row,size_t col) {
    std::cout<<"\x1b[" << row << ";" << col << "H"<<std::flush;
    set_cursor_pos({row, col});
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
void move_cursor(direction direction) {

}

/*TODO
 * - fix this piece of shit
 *      - gets random stuff from keyboard mixed in
 *      - fix misleading names of cursor_pos and get_cursor_pos
 *      - save get_cursor_pos to variable instead of rerunning
 *      - add comments
 *      - nvm it doesnt work at all
 */
// inline std::array<int ,2> get_cursor_pos() {
//     std::array<int,2> cursor_pos = {};
//     //print cursor pos to STDIN_FILENO
//     std::cout<<"\033[6n"<<std::flush;
//     u_char byte;
//     char raw_cords[32];
//     read(STDIN_FILENO, &byte, 1);
//     int i = 0;
//     debug_show_var("a");
//     while (byte != 'R') {
//         if (i > 31) {
//             break;
//         }
//         raw_cords[i] = byte;
//         i++;
//         read(STDIN_FILENO, &byte, 1);
//     }
//     raw_cords[i] = '\0';
//     sscanf(raw_cords, "\033[%d;%d",&cursor_pos[0], &cursor_pos[1]);;
//     return cursor_pos;
// }
