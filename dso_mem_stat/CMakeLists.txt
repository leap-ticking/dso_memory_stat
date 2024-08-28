cmake_minimum_required(VERSION 3.24)
project(dso_mem_stat)

add_compile_options(-g -O0 -ggdb -rdynamic)

add_library(dso_mem_stat SHARED
            mem_stat.cpp
        )

target_link_libraries(dso_mem_stat PUBLIC
        dl
        )

set_target_properties(dso_mem_stat PROPERTIES LINK_FLAGS "-shared")