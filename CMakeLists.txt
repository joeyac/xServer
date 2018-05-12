cmake_minimum_required(VERSION 3.10)
project(xServer C)

set(CMAKE_C_STANDARD 11)
add_definitions(-D_GNU_SOURCE)

link_libraries(pthread)

link_libraries(zlog)

# 动态处理
file(GLOB DYNAMIC_SOURCES dynamic/fcgi.c dynamic/fcgi.h dynamic/request.h)

# Server
file(GLOB SERVER_SOURCES ${DYNAMIC_SOURCES} csapp.h csapp.c server.c server.h utils.c utils.h)

# multi thread
file(GLOB THREAD_SOURCES threadUtils/xbuf.c threadUtils/xbuf.h threadUtils/xthread.h threadUtils/xthread.c)

# multi thread and multi process
file(GLOB PROCESS_SOURCES threadUtils/xbuf_p.c threadUtils/xthread_p.c threadUtils/xthread_p.h threadUtils/xbuf_p.h)

# 多进程xServer
add_executable(xServer main.c ${SERVER_SOURCES})

# 多线程xServer, 自己实现生产者消费者队列
add_executable(xServer-thread main-thread.c ${SERVER_SOURCES} ${THREAD_SOURCES})

# 多进程多线程xServer，设置cpu亲和性
add_executable(xServer-final main-final.c ${SERVER_SOURCES} ${PROCESS_SOURCES})
target_link_libraries(xServer-final rt)

add_executable(adder web/cgi-bin/adder.c csapp.h csapp.c)