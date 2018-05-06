//
// Created by xjw on 4/27/18.
//

#include <sched.h>
#include "utils.h"

zlog_category_t *zlogCategory;

void initLogger() {
    int rc;
    rc = zlog_init("zlog.conf");
    if (rc) app_error("init zlog failed.");
    zlogCategory = zlog_get_category("my_cat");
    if (!zlogCategory) {
        zlog_fini();
        app_error("get cat failed.");
    }
}

void testLogger() {
    DEBUG("DEBUG test");
    INFO("INFO test");
    NOTICE("NOTICE test");
    WARN("WARN test");
    ERROR("ERROR test");
    FATAL("FATAL test");
}

void help(char *name) {
    printf("simple HTTP server by xjw.\n\tusage: %s -p [port] -r [root_dir] -g [cgi_root_dir] -k [cgi_key] -w [workers] -c [worker_connects]\n",
           basename(name));
    puts("\t\tdefault port is 10000 and root dir is your current working directory.");
    puts("\t\tdefault cgi root dir is \"[root_dir]/cgi-bin/\" and cgi key is \"cgi-bin\", mapped cgi_key/* -> cgi_root/*.");
    puts("\t\tdefault workers is 4 and worker_connects is 1024.");
    exit(0);
}

void parseCmd(int argc, char *argv[]) {
    int c;
    // cgi_key = "cgi-bin"
    // cgi_key/* -> cgi_root/*
    config.cgi_key = "cgi-bin";
    config.cgi_root = "";
    config.root = getenv("PWD");
    config.port = 10000;
    config.workers = 4;
    config.worker_conn = 1024;
    //Parsing the command line arguments
    while ((c = getopt(argc, argv, "r:p:w:c:g:k:")) != -1)
        switch (c) {
            case 'r':
                config.root = optarg;
                break;
            case 'p':
                config.port = atoi(optarg);
                break;
            case 'w':
                config.workers = atoi(optarg);
                break;
            case 'c':
                config.worker_conn = atoi(optarg);
                break;
            case 'g':
                config.cgi_root = optarg;
                break;
            case 'k':
                config.cgi_key = optarg;
                break;
            default:
                help(argv[0]);
        }
    if (config.workers > get_nprocs()) {
        fprintf(stderr, "can not assign worker numbers greater then CPU counts, reset workers=%d\n", get_nprocs());
        config.workers = get_nprocs();
    }
    if (config.cgi_root == "") {
        char *s = "cgi-bin";
        char *sn = (char *) malloc(strlen(s) + strlen(config.root));
        pathJoin(sn, config.root);
        pathJoin(sn, s);
        config.cgi_root = sn;
    }
    if (*config.root != '/') {
        char *sn = (char *) malloc(strlen(config.root) + strlen(getenv("PWD")));
        pathJoin(sn, getenv("PWD"));
        pathJoin(sn, config.root);
        config.root = sn;
    }
    if (*config.cgi_root != '/') {
        char *sn = (char *) malloc(strlen(config.cgi_root) + strlen(getenv("PWD")));
        pathJoin(sn, getenv("PWD"));
        pathJoin(sn, config.cgi_root);
        config.cgi_root = sn;
    }
}

void printConfig() {
    printf("port: %d\n", config.port);
    printf("root: %s\n", config.root);
    printf("cgi-root: %s\n", config.cgi_root);
    printf("cgi-key: %s\n", config.cgi_key);
    printf("workers: %d\n", config.workers);
    printf("worker conns: %d\n", config.worker_conn);
}

static void stopAndExit() {
    zlog_fini();
    exit(0);
}

void stopHandler(int a) {
    INFO("Server has received stop signal: %d.\n", a);
    stopAndExit();
}

void ctrlHandler(int a) {
    INFO("You have press ctrl+c to exit: %d.\n", a);
    stopAndExit();
}

void childHandler(int a) {
    int stat;
    pid_t pid;
    pid = Wait(&stat);
    if (VERBOSE) INFO("%d: kill zombie process: %d", a, pid);
}

void sgUserHandler(int a) {
    ERROR("[main process] sched set affinity error, send exit signal to others...");
    kill(0, SIGTERM);
    stopAndExit();
}


void pathJoin(char *filename, char *append) {
    if (strlen(append) == 0) return;
    int flag = (filename[strlen(filename) - 1] == '/') + (append[0] == '/');
    size_t len = strlen(append);
    char *ptr = strstr(append, "?");
    if (ptr) len = (ptr - append) - 1;
    if (flag == 2) {
        strncat(filename, append + 1, len);
    } else if (flag == 1) {
        strncat(filename, append, len + 1);
    } else {
        strcat(filename, (const char *) "/");
        strncat(filename, append, len);
    }
}

static int php_htoi(char *s) {
    int value, c;
    c = ((unsigned char *) s)[0];
    if (isupper(c))
        c = tolower(c);
    value = (c >= '0' && c <= '9' ? c - '0' : c - 'a' + 10) * 16;

    c = ((unsigned char *) s)[1];
    if (isupper(c))
        c = tolower(c);
    value += c >= '0' && c <= '9' ? c - '0' : c - 'a' + 10;
    return (value);
}

int decode(char *str, size_t len) {
    char *dest = str;
    char *data = str;

    while (len--) {
        if (*data == '+') {
            *dest = ' ';
        } else if (*data == '%' && len >= 2 && isxdigit((int) *(data + 1)) && isxdigit((int) *(data + 2))) {
            *dest = (char) php_htoi(data + 1);
            data += 2;
            len -= 2;
        } else {
            *dest = *data;
        }
        data++;
        dest++;
    }
    *dest = '\0';
    return (int) (dest - str);
}

void Sched_setaffinity(int cpu_number) {
    cpu_set_t set;
    CPU_ZERO(&set);
    CPU_SET(cpu_number, &set);
    if (sched_setaffinity(0, sizeof(set), &set) < 0) {
        ERROR("[child process] unable to set affinity of process");
        kill(getppid(), SIGUSR1);   // 向父进程发送一个自定义信号
        exit(0);
    }

}
