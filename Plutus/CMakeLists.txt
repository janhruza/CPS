cmake_minimum_required(VERSION 4.3)
project(Plutus C)

set(CMAKE_C_STANDARD 23)

add_executable(Plutus main.c
        inc/Mortgage.h
        src/Mortgage.c
        inc/Loan.h
        src/Loan.c
        inc/Transaction.h
        src/Transaction.c
        inc/Config.h
        src/Config.c)

target_link_libraries(Plutus m) # libm math library