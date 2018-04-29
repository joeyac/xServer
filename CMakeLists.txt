cmake_minimum_required(VERSION 3.10)
project(xServer C)

set(CMAKE_C_STANDARD 11)

link_libraries(pthread)

link_libraries(zlog)

link_libraries(hiredis)

add_executable(xServer main.c csapp.h csapp.c server.c server.h utils.c utils.h )
add_executable(adder web/cgi-bin/adder.c csapp.h csapp.c)
add_executable(threading test/threading.c utils.c utils.h csapp.h csapp.c)