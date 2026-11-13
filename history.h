//
// Created by kacper on 11/10/26.
//

#ifndef PIGEONCLI_HISTORY_H
#define PIGEONCLI_HISTORY_H
#include <string>

namespace history {
    void display();
    void reset();
    void next();
    void previous();
    void open_read();
    void open_write();
    void close();
    void save_file();
    void save(const std::string& line);
}


#endif //PIGEONCLI_HISTORY_H
