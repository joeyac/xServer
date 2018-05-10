//
// Created by xjw on 5/9/18.
//
#include "../utils.h"
#include "xbuf_p.h"


/* 注意和xbuf.c中Sem_init以及pthread_mutex_init的参数区别 */
void buf_init(buf_p pf, size_t n) {
//    pf->buf = Calloc(n, sizeof(int)); 注意这里，需要用mmap去申请内存
//    pf->buf = create_shared_memory(config.buf_queue_size * sizeof(int));
    pf->n = n;
    pf->head = pf->tail = 0;
    pthread_mutex_init(&(pf->mutex), pthreadMutexattr);
    Sem_init(&(pf->full), 1, (unsigned int) n); // full=0代表无法再放入数据
    Sem_init(&(pf->empty), 1, 0);               // empty=0代表无法取出数据
}

void buf_push(buf_p pf, int data) {
    P(&(pf->full));
    LOCK(pf->mutex);
    pf->buf[(++pf->tail) % (pf->n)] = data;
    UNLOCK(pf->mutex);
    V(&(pf->empty));
}

void buf_pop(buf_p pf, int *data) {
    P(&(pf->empty));
    LOCK(pf->mutex);
    *data = pf->buf[(++pf->head) % (pf->n)];
    UNLOCK(pf->mutex);
    V(&(pf->full));
}