//
// Created by xjw on 5/9/18.
//
#include "threadUtils/xthread_p.h"

buf_p queue;
xthread_p workers;
pthread_mutexattr_t *pthreadMutexattr;
pthread_mutex_t *accept_mutex;

void *process_exit_func;

char process_name[64];
int *process_id;

void main_init();

void worker_init();

void exitMain();

void exitWorker();

void registerMainSignal();

void registerWorkerSignal();

int main(int argc, char **argv) {
    parseCmd(argc, argv);
    main_init();
    INFO("multi process-thread mode");

    /*  变量定义 */
    int main_pid = -1;
    int listenFd;

    /* 打开监听端口 */
    listenFd = Open_listenfd(config.port);
    INFO("Start listen on port: %d", config.port);

    /* 创建子进程 */
    pid_t cur_pid;
    for (int i = 0; i < config.workers; i++)
        if ((cur_pid = Fork()) == 0) {    /* child process */
            main_pid = i;
            Free(process_id);
            Sched_setaffinity(i);
            break;
        } else process_id[i] = cur_pid;

    if (-1 == main_pid) { /* main process */
        /* 设置主进程的退出函数指针 */
        process_exit_func = &exitMain;

        /* 注册主进程的监听信号 */
        registerMainSignal();

        int remain_restart_count = 4;
        pid_t sub_exit_process, sub_new_process;
        int status;
        while (1) {
            sub_exit_process = Wait(&status);
            INFO("found sub process %d exited", sub_exit_process);
            if (remain_restart_count-- > 0) {
                if ((sub_new_process = Fork()) == 0) {
                    for (int i = 0; i < config.workers; i++)
                        if (process_id[i] == sub_exit_process)
                            main_pid = i;
                    Free(process_id);
                    Sched_setaffinity(main_pid);
                    break;
                } else {
                    WARN("restart worker process %d->%d", sub_exit_process, sub_new_process);
                    for (int i = 0; i < config.workers; i++)
                        if (process_id[i] == sub_exit_process)
                            process_id[i] = sub_new_process;
                }
            } else {
                ERROR("no remain restart count, process exit...");
                exitMain();
                break;
            }

        }
    }
    if (main_pid >= 0) {/* child process */
        /* 设置子进程的退出处理函数指针 */
        process_exit_func = &exitWorker;

        /* 注册子进程的监听信号 */
        registerWorkerSignal();

        /* 初始化子进程 */
        worker_init();

        /* 创建消费者线程 */
        sprintf(process_name, "worker%d", main_pid + 1);
        create_thread(workers, 0, config.worker_conn, xconsumer, "xconsumer");

        int connFd;
        char hostname[MAXLINE], port[MAXLINE];
        socklen_t clientLen;
        struct sockaddr_storage clientAddr;

        while (1) {
            clientLen = sizeof(clientAddr);
            connFd = Accept(listenFd, (SA *) &clientAddr, &clientLen);
            Getnameinfo((SA *) &clientAddr, clientLen, hostname, MAXLINE, port, MAXLINE, 0);
            if (VERBOSE) INFO("Accept connection from (%s, %s)\n", hostname, port);
            buf_push(queue, connFd);
            if (VERBOSE) INFO("Fd %d in queue", connFd);
        }
    }

    exit(0);
}

void main_init() {
    process_exit_func = NULL;
    printConfig();
    initLogger();

    /* 动态分配id的内存 */
    process_id = Malloc(sizeof(int) * config.workers);

    /* 设置锁的进程间共享属性 */
    pthreadMutexattr = create_shared_memory(sizeof(pthread_mutexattr_t));
    pthread_mutexattr_init(pthreadMutexattr);
    pthread_mutexattr_setpshared(pthreadMutexattr, 1);

    /* 初始化全局accept锁 */
    accept_mutex = create_shared_memory(sizeof(pthread_mutex_t));
    pthread_mutex_init(accept_mutex, pthreadMutexattr);
}

void worker_init() {
    /* create shared memory for queue & thread pointer */
    queue = create_shared_memory(sizeof(buf_t));
    workers = create_shared_memory(config.worker_conn * sizeof(xthread_t));

    /* init signal so that threads can be create */
    for (int i = 0; i < config.worker_conn; i++)
        Sem_init(&(workers[i].closed), 1, 1);

    /* init buf queue */
    buf_init(queue, (size_t) (config.buf_queue_size));
    queue->buf = create_shared_memory(config.buf_queue_size * sizeof(int));
}


void registerMainSignal() {
    /* ctrl + c 信号 */
    Signal(SIGINT, ctrlHandler);
    /* 停止信号 */
    Signal(SIGTERM, stopHandler);
    /* 处理子进程 */
//    Signal(SIGCHLD, childHandler);
}

void registerWorkerSignal() {
    /* ctrl + c 信号 */
    Signal(SIGINT, SIG_IGN);
    /* 停止信号 */
    Signal(SIGTERM, stopHandler);
}

void exitMain() {
    /* 退出子进程 */
    /* pid=0 将信号传给和目前进程相同进程组的所有进程 */
    for (int i = 0; i < config.workers; i++) {
        INFO("send exit signal to worker-%d", i);
        kill(process_id[i], SIGTERM);
    }
    int st;
    pid_t exit_pid;

    //Sleep(10); // 等待所有子进程退出
    while ((exit_pid = wait(&st)) >= 0) {
        INFO("exit process: %d", exit_pid);
    }


    /* 释放malloc内存 */
    INFO("release memory of malloc");
    Free(process_id);

    /* 释放共享内存 */
    INFO("release memory of mmap");
    Munmap(pthreadMutexattr, sizeof(pthread_mutexattr_t));
    Munmap(accept_mutex, sizeof(pthread_mutex_t));


    /* 停止日志 */
    INFO("clean self");
    zlog_fini();
    exit(0);
}

void exitWorker() {
    INFO("exit consumers");
    /* 退出所有线程 */
    for (int i = 0; i < config.worker_conn; i++) {
        LOCK(workers[i].mutex);
        workers[i].status = XTD_CANCELLING;
        UNLOCK(workers[i].mutex);
    }
    for (int i = 0; i < config.worker_conn; i++)
        buf_push(queue, -1);

    for (int i = 0; i < config.worker_conn; i++) {
        P(&workers[i].closed);
    }

    /* 释放共享内存 */
    INFO("release memory of mmap");
    Munmap(pthreadMutexattr, sizeof(pthread_mutexattr_t));
    Munmap(accept_mutex, sizeof(pthread_mutex_t));
    Munmap(workers, config.worker_conn * sizeof(xthread_t));

    /* queue中还有init时申请的动态长度int数组 */
    Munmap(queue->buf, (size_t) (config.buf_queue_size));
    /* 再释放共享内存中的queue指针 */
    Munmap(queue, sizeof(buf_t));

    /* 停止日志 */
    INFO("clean self");
    zlog_fini();
    exit(0);
}