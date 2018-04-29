//
// Created by xjw on 4/27/18.
//

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
    printf("simple HTTP server by xjw.\n  usage:\n  ./%s -p [port] -r [root_dir] -w [workers] -c [worker_connects]\n", name);
    puts("  default port is 10000 and root dir is your current working directory.");
    puts("  default workers is 4 and worker_connects is 1024");
    exit(0);
}

void parseCmd(int argc, char *argv[]) {
    int c;
    config.root = getenv("PWD");
    config.port = 10000;
    config.workers = 4;
    config.worker_conn = 1024;
    //Parsing the command line arguments
    while ((c = getopt (argc, argv, "r:p:w:c:")) != -1)
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
            default:
                help(argv[0]);
        }
}

void printConfig() {
    printf("port: %d\n", config.port);
    printf("root: %s\n", config.root);
    printf("workers: %d\n", config.workers);
    printf("worker_conn: %d\n", config.worker_conn);
}

void stopHandler(int a) {
    INFO("Server has received stop signal.\n");
    zlog_fini();
    exit(0);
}

void ctrlHandler(int a) {
    INFO("You have press ctrl+c to exit.\n");
    zlog_fini();
    exit(0);
}
