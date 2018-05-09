//
// Created by xjw on 5/7/18.
//

#ifndef THREAD_CONTROL_CBUF_H
#define THREAD_CONTROL_CBUF_H

#include <pthread.h>
#include <semaphore.h>
#include "../csapp.h"

/* 生产者消费者队列 */
typedef struct {
    int *buf;                       /* 存储放入的数据 */
    volatile int head;              /* 头结点， 第一个元素是buf[(head+1) % n] */
    volatile int tail;              /* 尾节点， 最后一个元素是buf[tail % n] */
    size_t n;                       /* 元素个数 */
    pthread_mutex_t mutex;          /* 需要往buf增删数据,锁对buf的访问 */
    sem_t full;                     /* buf满信号 */
    sem_t empty;                    /* buf空信号 */
} buf_t, *buf_p;

#define BUF_EMPTY 0
#define BUF_MIDDLE 1
#define BUF_FULL 2

#define LOCK(x) pthread_mutex_lock(&((x)))
#define UNLOCK(x) pthread_mutex_unlock(&((x)))

void buf_init(buf_p pf, size_t n);

void buf_push(buf_p pf, int data);

void buf_pop(buf_p pf, int *data);


#endif //THREAD_CONTROL_CBUF_H
