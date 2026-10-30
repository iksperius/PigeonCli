//
// Created by kacper on 10/26/26.
//
#include "ansi_esc_helper.h"

#include <iostream>
#include <ostream>
#include "terminal.h"
namespace ansi {
    void clear_from_cur_pos() {
        std::cout<<"\033[0K"<<std::flush;
    }

    void move_cursor(const direction direction) {
        CursorPos cursor_pos = get_cursor_pos();
        switch (direction) {
            case UP:
                std::cout<<"\033[A"<<std::flush;
                cursor_pos.y += 1;
                break;
            case DOWN:
                std::cout<<"\033[B"<<std::flush;
                cursor_pos.y -= 1;
                break;
            case RIGHT:
                std::cout<<"\033[C"<<std::flush;
                cursor_pos.x += 1;
                break;
            case LEFT:
                std::cout<<"\033[D"<<std::flush;
                cursor_pos.x -= 1;
                break;
        }
        set_cursor_pos(cursor_pos);
    }
    void move_cursor(size_t row,size_t col) {
        std::cout<<"\x1b[" << row << ";" << col << "H"<<std::flush;
        set_cursor_pos(CursorPos(row,col));
    };
    void move_cursor(cursor_action action) {
        static CursorPos cursor_pos_buffer{};
        if (action == cursor_action::SAVE) {
            std::cout<<"\x1b[s"<<std::flush;
            cursor_pos_buffer = get_cursor_pos();
            return;
        }
        if (action == cursor_action::LOAD) {
            set_cursor_pos(cursor_pos_buffer);
            std::cout<<"\x1b[u"<<std::flush;
            return;
        }
    }
    void beep() {
        std::cout<<"\07" <<std::flush;
    }
    void move_line(direction direction, size_t col) {
        //need to use ansi escape character cuz we dont track y
        // CursorPos cursor_pos_buffer = get_cursor_pos();
        // switch (direction) {
        //     case UP:
        //         cursor_pos_buffer.y += 1;
        //         cursor_pos_buffer.x = col;
        //         break;
        //     case DOWN:
        //         cursor_pos_buffer.y -= 1;
        //         cursor_pos_buffer.x = 0;
        //         break;
        //     default:
        //         break;
        //         set_cursor_pos(cursor_pos_buffer);
        switch (direction) {
            case UP:
                std::cout<<"\033[F"<<std::flush;
                std::cout<<"\033["<<col <<"G"<< std::flush;
                break;
            case DOWN:
                std::cout<<"\033[E"<<std::flush;
                break;
            default:
                break;
        }
        }
    }
/*TODO
 * - constantly track cursor position instead of outputing it when needed (prevents miskeyboard imputs)
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
