cmake_minimum_required(VERSION 4.3)
project(ProxyShell C)

set(CMAKE_C_STANDARD 23)

add_executable(ProxyShell main.c)

set_target_properties(ProxyShell PROPERTIES OUTPUT_NAME proxsh)
