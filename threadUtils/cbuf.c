//
// Created by xjw on 5/7/18.
//

#include "cbuf.h"


void buf_init(buf_p pf, size_t n) {
    pf->buf = Calloc(n, sizeof(int));
    pf->n = n;
    pf->head = pf->tail = 0;
    pthread_mutex_init(&(pf->mutex), NULL);
    Sem_init(&(pf->full), 0, (unsigned int) n); // full=0代表无法再放入数据
    Sem_init(&(pf->empty), 0, 0);               // empty=0代表无法取出数据
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
