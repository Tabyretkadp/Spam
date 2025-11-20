cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

project(Spam VERSION 1.0 LANGUAGES CXX)

find_package(Td 1.8.56 REQUIRED)

add_executable(Spam main.cpp main.h)
target_link_libraries(Spam PRIVATE Td::TdStatic)
set_property(TARGET Spam PROPERTY CXX_STANDARD 14)
