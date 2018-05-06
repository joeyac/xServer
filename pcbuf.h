//
// Created by xjw on 5/6/18.
//

#ifndef XSERVER_PCBUF_H
#define XSERVER_PCBUF_H


/* Producer & consumers models */
#include "server.h"


/* 生产者消费者队列 */
typedef struct {
    int *buf;               /* 存储放入的数据 */
    int head;               /* 头结点， 第一个元素是buf[(head+1) % n] */
    int tail;               /* 尾节点， 最后一个元素是buf[tail % n] */
    int n;                   /* 元素个数 */
    sem_t mutex;            /* 需要往buf增删数据 */
    sem_t full;             /* buf满信号 */
    sem_t empty;            /* buf空信号 */
} pc_buf;


void pc_init(pc_buf *pf, unsigned int n);

void pc_free(pc_buf *pf);

void pc_add(pc_buf *pf, int item);

int pc_pop(pc_buf *pf);


/* 多线程并发相关 */
typedef struct {
    pthread_t pid;
    sem_t mutex;
} thread, *pthread;

/* 线程处理函数 */
void *proc_thread(void *arg);

/* 动态调整线程数量线程 */
void *adjust_thread(void *arg);

#endif //XSERVER_PCBUF_H
