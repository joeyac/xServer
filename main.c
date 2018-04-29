#include "server.h"
#include "utils.h"


int main(int argc, char **argv) {
    int listenFd, connFd;
    char hostname[MAXLINE], port[MAXLINE];
    socklen_t clientLen;
    struct sockaddr_storage clientAddr;

    signal(SIGINT, ctrlHandler);
    signal(SIGTERM, stopHandler);

    parseCmd(argc, argv);
    printConfig();
    initLogger();

    listenFd = Open_listenfd(config.port);
    //INFO("Start listen on port: %s%d%s", "\033[92m", config.port, "\033[0m");
    INFO("Start listen on port: %d", config.port);
    while (1) {
        clientLen = sizeof(clientAddr);
        connFd = Accept(listenFd, (SA *)&clientAddr, &clientLen);
        Getnameinfo((SA *) &clientAddr, clientLen, hostname, MAXLINE, port, MAXLINE, 0);
        INFO("Accept connection from (%s, %s)\n", hostname, port);
        doit(connFd);
        close(connFd);
    }
    exit(0);
}