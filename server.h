//
// Created by xjw on 4/27/18.
//

#ifndef XSERVER_SERVER_H
#define XSERVER_SERVER_H

/* for using strptime function, should define the two vars before include time.h */
#ifndef __USE_XOPEN
#define __USE_XOPEN
#endif
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <time.h>
#include <netinet/tcp.h>
#include "csapp.h"
#include "utils.h"
#include "dynamic/fcgi.h"


typedef struct tm tm_t;

void Im_rio_writen(int fd, void *usrbuf, size_t n);

void doit(int fd);

int read_requesthdrs(rio_t *rp, char *method, tm_t *ti, bool *tm_done, char *content_type);

int parse_uri(char *uri, char *filename, char *cgiargs);

void serve_static(int fd, char *filename, int filesize, char *method, time_t *request, tm_t *modify);

void get_filetype(char *filename, char *filetype);

void serve_default_dynamic(int fd, char *filename, char *cgiargs, char *method);

void
serve_dynamic(int fd, char *filename, char *cgiargs, unsigned int content_length, char *content_type, char *method);

void clienterror(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg);

#endif //XSERVER_SERVER_H
