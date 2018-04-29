//
// Created by xjw on 4/27/18.
//

#ifndef XSERVER_LOGGER_H
#define XSERVER_LOGGER_H

#include <stdbool.h>
#include "csapp.h"
#include "zlog.h"


/* logger wrappers */
extern zlog_category_t *zlogCategory;
void initLogger();
void testLogger();
#define DEBUG(args...) zlog_debug(zlogCategory, args)
#define INFO(args...) zlog_info(zlogCategory, args)
#define NOTICE(args...) zlog_notice(zlogCategory, args)
#define WARN(args...) zlog_warn(zlogCategory, args)
#define ERROR(args...) zlog_error(zlogCategory, args)
#define FATAL(args...) zlog_fatal(zlogCategory, args)

/* config file wrappers */
typedef struct {
    int port;
    char *root;
    int workers;
    int worker_conn;
} Config;
Config config;
void parseCmd(int argc, char *argv[]);
void printConfig();

/* sig handler functions */
void ctrlHandler(int a);
void stopHandler(int a);


/* path build help function */
void pathJoin(char *filename, char *append);


/* url decode function (zh_cn) */
int decode(char *str, size_t len);

#endif //XSERVER_LOGGER_H
