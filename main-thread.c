#include "threadUtils/xthread.h"

xthread_p mainThread;
xthread_p workers;
buf_p queue;

pthread_mutex_t mutex_cnt;
pthread_mutex_t workers_full, workers_empty;
volatile int workers_cnt = 0;

void usr1(int a);   // double workers or up one worker, signal 10
void usr2(int a);   // half workers or down one worker, signal 12


void *add_workers(void *vargp) {
    int new_cnt, need_add;
    /* initial create one workers */

    LOCK(mutex_cnt);
    workers_cnt = 1;
    create_thread(workers, 0, 1, xconsumer, "workers");
    UNLOCK(mutex_cnt);

    while (1) {
        LOCK(queue->mutex);
        need_add = (queue->tail - queue->head) > (queue->n / 2);
        UNLOCK(queue->mutex);

        if (need_add) {
            LOCK(mutex_cnt);

            new_cnt = workers_cnt * 2;

            if (new_cnt == 0) new_cnt = 1;
            if (new_cnt > config.workers * config.worker_conn) new_cnt = config.workers * config.worker_conn;

            if (new_cnt > workers_cnt) {
                create_thread(workers, workers_cnt, new_cnt, xconsumer, "workers");
                NOTICE("add workers: %d->%d", workers_cnt, new_cnt);
            }

            workers_cnt = new_cnt;

            UNLOCK(mutex_cnt);
        }
        sleep(1);
    }

}

void *del_workers(void *vargp) {
    int new_cnt, need_del;
    while (1) {
        LOCK(mutex_cnt);
        need_del = (queue->tail - queue->head) < (queue->n / 2);
        UNLOCK(mutex_cnt);

        if (need_del) {
            LOCK(mutex_cnt);

            new_cnt = workers_cnt / 2;

            if (new_cnt == 0) new_cnt = 1;

            if (new_cnt < workers_cnt) {
                cancel_thread(workers, new_cnt, workers_cnt, "workers");
                NOTICE("cancel workers: %d->%d", workers_cnt, new_cnt);
            }

            workers_cnt = new_cnt;

            UNLOCK(mutex_cnt);
        }
        sleep(1);
    }

}


int main(int argc, char **argv) {

    registerSignal();

    parseCmd(argc, argv);

    printConfig();
    initLogger();

    mainThread = create_shared_memory(sizeof(xthread_t));
    workers = create_shared_memory(config.workers * config.worker_conn * sizeof(xthread_t));
    queue = create_shared_memory(sizeof(buf_t));

    /* init buf queue*/
    buf_init(queue, (size_t) (config.workers * config.worker_conn));

    /* init signal so that threads can be create */
    Sem_init(&(mainThread[0].closed), 0, 1);
    for (int i = 0; i < config.workers * config.worker_conn; i++)
        Sem_init(&(workers[i].closed), 0, 1);

    /* create main thread */
    create_thread(mainThread, 0, 1, xproducer, "producer");


    /* create worker threads */
    workers_cnt = config.workers * config.worker_conn;
    create_thread(workers, 0, workers_cnt, xconsumer, "workers");

    /* signal usr for debug */
//    Signal(SIGUSR1, usr1);
//    Signal(SIGUSR2, usr2);

    /* init workers full and empty mutex */
//    pthread_mutex_init(&(workers_full), NULL);
//    pthread_mutex_init(&(workers_empty), NULL);

    /* adjust thread */
//    pthread_t adjust_t;
//    pthread_mutex_init(&mutex_cnt, NULL);
//    Pthread_create(&adjust_t, NULL, add_workers, NULL);
//    Pthread_create(&adjust_t, NULL, del_workers, NULL);

    /* main loop */
    while (1);
    exit(0);
}

void usr1(int a) {
    int new_cnt = workers_cnt * 2;
    if (new_cnt == 0) new_cnt = 1;
    if (new_cnt > config.workers * config.worker_conn) new_cnt = config.workers * config.worker_conn;

    WARN("start arrange consumer: %d->%d", workers_cnt, new_cnt);

    create_thread(workers, workers_cnt, new_cnt, xconsumer, "workers");

    WARN("end arrange consumer: %d->%d", workers_cnt, new_cnt);
    workers_cnt = new_cnt;
}


void usr2(int a) {
    int new_cnt = workers_cnt / 2;

    cancel_thread(workers, new_cnt, workers_cnt, "workers");

    workers_cnt = new_cnt;
}

