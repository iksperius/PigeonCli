//
// Created by kacper on 10/26/26.
//
#include "ansi_esc_helper.h"

#include <cmath>
#include <iostream>
#include <ostream>
#include <sys/ioctl.h>

#include "terminal.h"
namespace ansi {
    void clear_from_cur_pos() {
        std::cout<<"\033[0J"<<std::flush;
        move_cursor(SAVE);
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
        std::cout<<"\033[" << row << ";" << col << "H"<<std::flush;
        set_cursor_pos(CursorPos(row,col));
    };
    void move_cursor(cursor_action action) {
        static CursorPos cursor_pos_buffer{};
        if (action == cursor_action::SAVE) {
            std::cout<<"\0337"<<std::flush;
            cursor_pos_buffer = get_cursor_pos();
            return;
        }
        if (action == cursor_action::LOAD) {
            set_cursor_pos(cursor_pos_buffer);
            std::cout<<"\0338"<<std::flush;
            return;
        }
    }
    void beep() {
        std::cout<<"\07" <<std::flush;
    }
    void move_line(direction direction) {
        const winsize ws = get_ws();
        switch (direction) {
            case UP:
                std::cout<<"\033[F"<<std::flush;
                std::cout<<"\033["<<ws.ws_col<<"G"<< std::flush;
                set_cursor_pos(CursorPos(ws.ws_col,0));
                break;
            case DOWN:
                std::cout<<"\n\r"<<std::flush;
                set_cursor_pos(CursorPos(0,0));
                break;
            default:
                break;
            }
        }
    void scroll(direction direction) {
        switch (direction) {
            case UP:
                std::cout<<"\033[S"<<std::flush;
                break;
            case DOWN:
                std::cout<<"\033[T"<<std::flush;
                break;
            default:
                break;
        }
    }

    void beginning_of_line() {
        std::cout<<"\r"<<std::flush;
    }
}
