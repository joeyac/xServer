//
// Created by xjw on 5/7/18.
//

#include "xthread.h"

void xthread_init(xthread_p thread_p) {
    pthread_mutex_init(&(thread_p->mutex), NULL);
    thread_p->status = XTD_ENABLE;
}

void create_thread(xthread_p threads, int st, int ed, void *func, const char *info) {
    for (int i = st; i < ed; i++) {
        int *arg = (int *) Malloc(sizeof(int));
        *arg = i;
        P(&(threads[i].closed));
        xthread_init(&(threads[i]));
        Pthread_create(&threads[i].tid, NULL, func, arg);
        NOTICE("finish start thread %s %d job", info, i);
    }
}


void cancel_thread(xthread_p threads, int st, int ed, const char *info) {
    for (int i = st; i < ed; i++) {
        pthread_mutex_lock(&(threads[i].mutex));
        threads[i].status = XTD_CANCELLING;
        pthread_mutex_unlock(&(threads[i].mutex));
        NOTICE("finish cancel thread %s %d job", info, i);
    }
}


static void setName(const char *name, const int id) {
    char thread_name[128] = {0};
    sprintf(thread_name, "%s-%d", name, id);
    prctl(PR_SET_NAME, thread_name);
}


void *xproducer(void *vargp) {
    /* Set thread name for profiling and debugging */
    int id = *(int *) vargp;
    setName("xproducer", id);
    free(vargp);


    /*  变量定义 */
    int listenFd, connFd, status;
    char hostname[MAXLINE], port[MAXLINE];
    socklen_t clientLen;
    struct sockaddr_storage clientAddr;

    /* 打开监听端口 */
    listenFd = Open_listenfd(config.port);
    INFO("Start listen on port: %d", config.port);

    while (1) {
        /* 锁住线程状态再查询 */
        LOCK(mainThread[id].mutex);
        status = mainThread[id].status;
        UNLOCK(mainThread[id].mutex);

        /* 如果状态为需要取消， 修改线程状态 */
        if (XTD_CANCELLING == status) {
            LOCK(mainThread[id].mutex);
            mainThread[id].status = XTD_CANCELLED;
            UNLOCK(mainThread[id].mutex);
            break;
        }
        clientLen = sizeof(clientAddr);
        connFd = Accept(listenFd, (SA *) &clientAddr, &clientLen);
        Getnameinfo((SA *) &clientAddr, clientLen, hostname, MAXLINE, port, MAXLINE, 0);
        if (VERBOSE) INFO("Accept connection from (%s, %s)\n", hostname, port);

        buf_push(queue, connFd);
        if (VERBOSE) INFO("Fd %d in queue", connFd);
    }

    /* 发送已经关闭的信号，以便于重新创建新线程 */
    V(&mainThread[id].closed);
    return NULL;
}

void *xconsumer(void *vargp) {
    /* Set thread name for profiling and debugging */
    int id = *(int *) vargp;
    setName("xconsumer", id);
    free(vargp);

    /*  变量定义 */
    int connFd, status;

    while (1) {
        LOCK(workers[id].mutex);
        status = workers[id].status;
        UNLOCK(workers[id].mutex);

        /* 如果状态为需要取消， 修改线程状态 */
        if (XTD_CANCELLING == status) {
            LOCK(workers[id].mutex);
            workers[id].status = XTD_CANCELLED;
            UNLOCK(workers[id].mutex);
            break;
        }

        buf_pop(queue, &connFd);
        if (VERBOSE) INFO("consumer get Fd %d", connFd);
        doit(connFd);
        Close(connFd);
    }

    V(&workers[id].closed);
    return NULL;
}

void *create_shared_memory(size_t size) {
    // Our memory buffer will be readable and writable:
    int protection = PROT_READ | PROT_WRITE;

    // The buffer will be shared (meaning other processes can access it), but
    // anonymous (meaning third-party processes cannot obtain an address for it),
    // so only this process and its children will be able to use it:
    int visibility = MAP_ANONYMOUS | MAP_SHARED;

    // The remaining parameters to `mmap()` are not important for this use case,
    // but the manpage for `mmap` explains their purpose.
    return Mmap(NULL, size, protection, visibility, 0, 0);
}




