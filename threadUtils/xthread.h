//
// Created by xjw on 5/7/18.
//

#ifndef THREAD_CONTROL_XTHREAD_H
#define THREAD_CONTROL_XTHREAD_H

#include <semaphore.h>
#include <sys/prctl.h>
#include "../server.h"
#include "cbuf.h"


typedef struct {
    pthread_t tid;
    pthread_mutex_t mutex;     // 对status状态的锁
    sem_t closed;              // 是否关闭的锁
    volatile int status;
} xthread_t, *xthread_p;

extern xthread_p mainThread;
extern xthread_p workers;
extern buf_p queue;

void xthread_init(xthread_p thread_p);

void create_thread(xthread_p threads, int st, int ed, void *func, const char *info);  /* [st, ed) */
void cancel_thread(xthread_p threads, int st, int ed, const char *info);  /* [st, ed) */

void *create_shared_memory(size_t size);

#define XTD_ENABLE      0
#define XTD_CANCELLING  1
#define XTD_CANCELLED   2


void *xproducer(void *vargp);

void *xconsumer(void *vargp);


#endif //THREAD_CONTROL_XTHREAD_H
