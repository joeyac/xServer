#include "pcbuf.h"


int main(int argc, char **argv) {
    int listenFd, connFd;
    char hostname[MAXLINE], port[MAXLINE];
    socklen_t clientLen;
    struct sockaddr_storage clientAddr;
    /* ctrl + c 信号 */
    Signal(SIGINT, ctrlHandler);
    /* clion 发送的停止信号 */
    Signal(SIGTERM, stopHandler);
    /* 忽略epipe信号 */
    Signal(SIGPIPE, SIG_IGN);
    /* 处理子进程 */
    Signal(SIGCHLD, childHandler);
    /* 处理子进程信号用户信号1 */
    Signal(SIGUSR1, sgUserHandler);

    parseCmd(argc, argv);

    printConfig();
    initLogger();
    listenFd = Open_listenfd(config.port);
    INFO("Start listen on port: %d", config.port);

    while (1) {
        clientLen = sizeof(clientAddr);
        connFd = Accept(listenFd, (SA *) &clientAddr, &clientLen);
        Getnameinfo((SA *) &clientAddr, clientLen, hostname, MAXLINE, port, MAXLINE, 0);
        if (VERBOSE) INFO("Accept connection from (%s, %s)\n", hostname, port);
        int *fdp = (int *) Malloc(sizeof(int));
        *fdp = connFd;
        pthread_t tid;
        Pthread_create(&tid, NULL, proc_thread, (void *) fdp);
//        Close(connFd);
    }
    exit(0);
}

