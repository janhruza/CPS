cmake_minimum_required(VERSION 4.3)
project(Plutus C)

set(CMAKE_C_STANDARD 23)

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
        src/Engine/ExpenseTracker.c)

target_link_libraries(Plutus m) # libm math library