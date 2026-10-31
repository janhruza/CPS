cmake_minimum_required(VERSION 3.25)
project(Plutus C)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_compile_options(-Wall -Wextra -Wpedantic)

add_executable(Plutus main.c
        inc/Engine/Mortgage.h
        src/Engine/Mortgage.c
        inc/Engine/Loan.h
        src/Engine/Loan.c
        inc/Engine/Transaction.h
        src/Engine/Transaction.c
        inc/Config.h
        src/Config.c
        inc/Engine/ExpenseTracker.h
        inc/MicroApps.h
        src/MicroApps.c
        src/Engine/ExpenseTracker.c
        inc/Menu.h
        src/Menu.c
        src/Core.c
        inc/Core.h
        inc/Engine/Investment.h
        src/Engine/Investment.c
        inc/Engine/PlutusFile.h
        src/Engine/PlutusFile.c
        inc/Ansi.h)

target_include_directories(Plutus PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(Plutus m) # libm math library