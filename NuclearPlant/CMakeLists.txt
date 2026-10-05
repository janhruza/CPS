cmake_minimum_required(VERSION 4.3)
project(NuclearPlant C)

set(CMAKE_C_STANDARD 23)

add_executable(NuclearPlant main.c
        inc/Reactor.h
        src/Reactor.c
        inc/messages.h
        inc/Version.h
        src/Version.c
        inc/PowerPlant.h
        src/PowerPlant.c
        inc/commands.h
        src/commands.c
)
