cmake_minimum_required(VERSION 4.3)
project(ProxyShell C)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_FLAGS -Wextra)

add_executable(ProxyShell main.c
        inc/command.h
        inc/macros.h
        inc/commands.h
        src/commands.c
        inc/types.h
        src/command.c)

set_target_properties(ProxyShell PROPERTIES OUTPUT_NAME proxsh)
