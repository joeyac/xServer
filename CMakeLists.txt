cmake_minimum_required(VERSION 3.10)
project(xServer C)

set(CMAKE_C_STANDARD 11)
add_definitions(-D_GNU_SOURCE)

link_libraries(pthread)

link_libraries(zlog)

link_libraries(hiredis)

# 多进程xServer
add_executable(xServer main.c csapp.h csapp.c server.c server.h utils.c utils.h)

# 多线程xServer, 利用redis做生产者消费者队列，动态控制线程数量
add_executable(xServer-thread main-thread.c csapp.h csapp.c server.c server.h utils.c utils.h threadUtils/cbuf.c threadUtils/cbuf.h threadUtils/xthread.h threadUtils/xthread.c)

add_executable(adder web/cgi-bin/adder.c csapp.h csapp.c)

add_executable(threading test/threading.c utils.c utils.h csapp.h csapp.c)
target_link_libraries(threading event)