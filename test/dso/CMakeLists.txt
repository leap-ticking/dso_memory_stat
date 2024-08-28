cmake_minimum_required(VERSION 3.24)
project(test_dso)

add_library(test_dso1 SHARED
        test_dso1.cpp
        )

add_library(test_dso2 SHARED
        test_dso2.cpp
        )