#include "server.h"
#include "utils.h"


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

    parseCmd(argc, argv);
    printConfig();
    initLogger();

    listenFd = Open_listenfd(config.port);
    //INFO("Start listen on port: %s%d%s", "\033[92m", config.port, "\033[0m");
    INFO("Start listen on port: %d", config.port);
    while (1) {
        clientLen = sizeof(clientAddr);
        connFd = Accept(listenFd, (SA *) &clientAddr, &clientLen);
        Getnameinfo((SA *) &clientAddr, clientLen, hostname, MAXLINE, port, MAXLINE, 0);
        if (VERBOSE) INFO("Accept connection from (%s, %s)\n", hostname, port);
        if (Fork() == 0) {
            /* 忽略epipe信号 */
            Signal(SIGPIPE, SIG_IGN);
            Close(listenFd);
            doit(connFd);
            Close(connFd);
            // DEBUG("exit process: %d", getpid());
            exit(0);
        }
        Close(connFd);
    }
    exit(0);
}