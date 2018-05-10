//
// Created by xjw on 5/9/18.
//
#include "xthread_p.h"

void xthread_init(xthread_p thread_p) {
    pthread_mutex_init(&(thread_p->mutex), pthreadMutexattr);
    thread_p->status = XTD_ENABLE;
}

void create_thread(xthread_p threads, int st, int ed, void *func, const char *info) {
    for (int i = st; i < ed; i++) {
        int *arg = (int *) Malloc(sizeof(int));
        *arg = i;
        P(&(threads[i].closed));
        xthread_init(&(threads[i]));
        Pthread_create(&threads[i].tid, NULL, func, arg);
    }
    NOTICE("finish start %s[%d,%d] job", info, st, ed);
}

static void setName(const char *name, const int id) {
    char thread_name[128] = {0};
    sprintf(thread_name, "%s-consumer%d", name, id);
    prctl(PR_SET_NAME, thread_name);
}

void *xconsumer(void *vargp) {
    Pthread_detach(Pthread_self());
    /* Set thread name for profiling and debugging */
    int id = *(int *) vargp;
    setName(process_name, id);
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
        if (connFd < 0) break;  // exit
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




