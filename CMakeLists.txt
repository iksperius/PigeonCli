cmake_minimum_required(VERSION 4.4)
project(PigeonCli)

set(CMAKE_CXX_STANDARD 20)

add_executable(PigeonCli main.cpp
        terminal.cpp
        terminal.h
        parser.cpp
        parser.h
        execute.cpp
        execute.h
        pigeon.cpp
        pigeon.h
)
