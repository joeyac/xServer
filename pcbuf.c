//
// Created by xjw on 5/6/18.
//

#include "pcbuf.h"

void pc_init(pc_buf *pf, unsigned int n) {
    pf->buf = Calloc(n, sizeof(int));
    pf->n = n;
    pf->head = pf->tail = 0;
    Sem_init(&pf->mutex, 0, 1);     /* 初值为1， 可以访问buf */
    Sem_init(&pf->full, 0, n);      /* 代表是否满了，初始为空，所以信号量初值为n */
    Sem_init(&pf->empty, 0, 0);     /* 代表是否为空， 初始为空，所以信号量初值为0 */
}

void pc_free(pc_buf *pf) {
    Free(pf->buf);
}

void pc_add(pc_buf *pf, int item) {
    P(&pf->full);
    P(&pf->mutex);
    pf->buf[(++pf->tail) % (pf->n)] = item;
    V(&pf->mutex);
    V(&pf->empty);
}

int pc_pop(pc_buf *pf) {
    int item;
    P(&pf->empty);
    P(&pf->mutex);
    item = pf->buf[(++pf->head) % (pf->n)];
    V(&pf->mutex);
    V(&pf->full);
    return item;
}

void *proc_thread(void *arg) {
    Pthread_detach(pthread_self());
    int fd = (*(int *) arg);
    Free(arg); /* 防止内存泄露,释放 */
    NOTICE("%lx: fd = %d\n", pthread_self(), fd);
    doit(fd);
    NOTICE("%lx: close fd = %d\n", pthread_self(), fd);
    close(fd);
}

void *adjust_thread(void *arg) {

    return NULL;
}

