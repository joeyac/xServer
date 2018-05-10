//
// Created by xjw on 5/9/18.
//

#ifndef XSERVER_XTHREAD_P_H
#define XSERVER_XTHREAD_P_H


#include "xbuf_p.h"
#include "../server.h"
#include <sys/prctl.h>
#include <sys/types.h>
#include <unistd.h>

typedef struct {
    pthread_t tid;
    pthread_mutex_t mutex;     // 对status状态的锁
    sem_t closed;              // 是否关闭的锁
    volatile int status;
} xthread_t, *xthread_p;

extern xthread_p workers;
extern pthread_mutexattr_t *pthreadMutexattr;
extern buf_p queue;
extern char process_name[64];

void xthread_init(xthread_p thread_p);

void create_thread(xthread_p threads, int st, int ed, void *func, const char *info);  /* [st, ed) */

void *create_shared_memory(size_t size);

#define XTD_ENABLE      0
#define XTD_CANCELLING  1
#define XTD_CANCELLED   2

void *xconsumer(void *vargp);

#endif //XSERVER_XTHREAD_P_H
