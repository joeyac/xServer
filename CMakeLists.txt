cmake_minimum_required(VERSION 3.10)
project(xServer C)

set(CMAKE_C_STANDARD 11)
add_definitions(-D_GNU_SOURCE)

link_libraries(pthread)

link_libraries(zlog)

link_libraries(hiredis)

# 多进程xServer
add_executable(xServer main.c csapp.h csapp.c server.c server.h utils.c utils.h)

# 多线程xServer, 自己实现生产者消费者队列
add_executable(xServer-thread main-thread.c csapp.h csapp.c server.c server.h utils.c utils.h threadUtils/xbuf.c threadUtils/xbuf.h threadUtils/xthread.h threadUtils/xthread.c)

# 多进程多线程xServer，设置cpu亲和性
add_executable(xServer-final main-final.c csapp.h csapp.c server.c server.h utils.c utils.h threadUtils/xbuf_p.c threadUtils/xthread_p.c threadUtils/xthread_p.h threadUtils/xbuf_p.h)
target_link_libraries(xServer-final rt)

add_executable(adder web/cgi-bin/adder.c csapp.h csapp.c)

add_executable(threading test/threading.c utils.c utils.h csapp.h csapp.c)
target_link_libraries(threading event)