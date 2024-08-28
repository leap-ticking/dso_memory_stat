cmake_minimum_required(VERSION 3.24)
project(main_prog)

add_executable(main_prog
        main.cpp
        )

target_link_libraries(main_prog PUBLIC
        test_dso1
        test_dso2
)