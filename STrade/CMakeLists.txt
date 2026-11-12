cmake_minimum_required(VERSION 4.3)
project(STrade C)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_FLAGS -Wextra)

add_executable(STrade main.c
        inc/macros.h
        inc/types.h
        inc/Game.h
        inc/Asset.h
        inc/Holding.h
        inc/Game.h
        src/Asset.c
)

set_target_properties(STrade PROPERTIES OUTPUT_NAME strade)
