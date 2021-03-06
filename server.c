//
// Created by xjw on 4/27/18.
//

#include "server.h"

const char *time_fmt = "%a, %d %b %Y %T %Z";


/* improved rio written, handle EPIPE signal */
void Im_rio_writen(int fd, void *usrbuf, size_t n) {
    if (rio_writen(fd, usrbuf, n) != n) {
        if (errno == EPIPE) {
            if (VERBOSE) WARN("%s: connection ended by customer.", strerror(errno));
            return;
            exit(0);
        }
        unix_error("Rio_writen error");
    }
}

ssize_t Im_rio_readlineb(rio_t *rp, void *usrbuf, size_t maxlen) {
    ssize_t rc;

    if ((rc = rio_readlineb(rp, usrbuf, maxlen)) < 0) {
        if (errno == EAGAIN || errno == EPIPE) {
            if (VERBOSE) WARN("%s: Im_rio_readlineb ended.", strerror(errno));
            return -1;
        }

        unix_error("IM Rio_readlineb error");
    }
    return rc;
}

/*
 * doit - handle one HTTP request/response transaction
 * 对静态文件有get和head方法
 * 对动态文件有get和post和head方法
 */
/* $begin doit */

void doit(int fd) {
    ssize_t retv;
    int is_static;      /* 是否为静态文件 */
    struct stat sbuf;  /* 用于获得文件的信息 */
    char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
    char filename[MAXLINE], cgiargs[MAXLINE], content_type[MAXLINE], file_type[MAXLINE];
    struct tm tm_request, tm_modify;
    time_t tt_request = 0;
    bool tm_done;

    rio_t rio;

    struct timeval timeout;
    timeout.tv_sec = 10;
    timeout.tv_usec = 0;
    Setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, (char *) &timeout, sizeof(timeout));
    setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, (char *) &timeout, sizeof(timeout));

//    int keepalive = 1; // 开启keepalive属性
//    int keepidle = 5; // 如该连接在60秒内没有任何数据往来,则进行探测
//    int keepinterval = 5; // 探测时发包的时间间隔为5 秒
//    int keepcount = 2; // 探测尝试的次数。如果第1次探测包就收到响应了,则后2次的不再发。
//    setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, (void *)&keepalive , sizeof(keepalive ));
//    setsockopt(fd, SOL_TCP, TCP_KEEPIDLE, (void*)&keepidle , sizeof(keepidle ));
//    setsockopt(fd, SOL_TCP, TCP_KEEPINTVL, (void *)&keepinterval , sizeof(keepinterval ));
//    setsockopt(fd, SOL_TCP, TCP_KEEPCNT, (void *)&keepcount , sizeof(keepcount ));


    /* Read request line and headers */
    Rio_readinitb(&rio, fd);    /* 对rio进行初始化 */
//    DEBUG("RIO read first line start: fd %d", fd);
//    Rio_readlineb(&rio, buf, MAXLINE);       /* 读取一行数据 */
    retv = Im_rio_readlineb(&rio, buf, MAXLINE);
//    DEBUG("Im_rio_readlineb: %ld", retv);
    if (retv == -1)
        return;

    if (retv == 0) {

//        DEBUG("len: %ld data: %s", strlen(buf), buf);
        return;
    }

    if (strlen(buf) == 0) {
        if (VERBOSE) WARN("empty request header");
        return;
    }

    sscanf(buf, "%s %s %s", method, uri, version);
    if (VERBOSE) INFO("[request %s %s] %s", version, method, uri);

    /* strcasecmp 忽略大小写比较字符串， 相等返回0 */
    if (!(strcasecmp(method, "GET") == 0 || strcasecmp(method, "POST") == 0 || strcasecmp(method, "HEAD") == 0)) {
        clienterror(fd, method, "501", "Not Implemented",
                    "XServer does not implement this method"); /* 暂时只支持GET POST 方法 */
        return;
    }
    int param_len = read_requesthdrs(&rio, method, &tm_request, &tm_done, content_type);
    if (param_len == -1)
        return;

    if (tm_done) tt_request = mktime(&tm_request);  // 如果有If-Modified-Since字段

    if (param_len) {
        retv = Rio_readnb(&rio, buf, (size_t) param_len);
        buf[retv] = '\0';
    }

    /* Parse URI from GET request */
    is_static = parse_uri(uri, filename, cgiargs);

    /* Decode filename by url_decode from php */
    decode(filename, strlen(filename));


    if (stat(filename, &sbuf) < 0) {
        clienterror(fd, filename, "404", "Not found",
                    "XServer couldn't find this file");
        return;
    }

    if (is_static) { /* Serve static content */
        if (!(S_ISREG(sbuf.st_mode)) || !(S_IRUSR & sbuf.st_mode)) {
            clienterror(fd, filename, "403", "Forbidden",
                        "xServer couldn't read the file");
            return;
        }
        tm_modify = *gmtime(&sbuf.st_mtim.tv_sec);
        serve_static(fd, filename, (int) sbuf.st_size, method, &tt_request, &tm_modify);
    } else { /* Serve dynamic content */
        if (VERBOSE) INFO("[serve dynamic]: %s", filename);
        if (!(S_ISREG(sbuf.st_mode)) || !(S_IXUSR & sbuf.st_mode)) {
            clienterror(fd, filename, "403", "Forbidden",
                        "xServer couldn't run the CGI program");
            return;
        }

        if (strcasecmp(method, "GET") == 0)
            serve_dynamic(fd, filename, cgiargs, (unsigned int) param_len, content_type, method);
        else
            serve_dynamic(fd, filename, buf, (unsigned int) param_len, content_type, method);
    }
}
/* $end doit */

/*
 * read_requesthdrs - read and parse HTTP request headers
 */
/* $begin read_requesthdrs */
int read_requesthdrs(rio_t *rp, char *method, tm_t *ti, bool *tm_done, char *content_type) {
    char buf[MAXLINE];
    char timeStr[MAXLINE];
    int len = 0;
    ssize_t retv;
    do {
//        Rio_readlineb(rp, buf, MAXLINE);
        retv = Im_rio_readlineb(rp, buf, MAXLINE);
        if (retv == -1) return -1;

        if (VERBOSE >= 2) INFO("%s", buf);
        if (strncasecmp(buf, "If-Modified-Since:", 18) == 0) { // 判断是否有If-Modified-Since字段
            sscanf(buf, "If-Modified-Since: %[^\n]", timeStr);
            strptime(timeStr, time_fmt, ti);
            *tm_done = true;
        }
        if (strncasecmp(buf, "Content-Type:", 13) == 0) {
            sscanf(buf, "Content-Type: %s", content_type);
        }

        if (strcasecmp(method, "POST") == 0 && strncasecmp(buf, "Content-Length:", 15) == 0)
            sscanf(buf, "Content-Length: %d", &len);
    } while (strcmp(buf, "\r\n"));
    return len;
}

/* $end read_requesthdrs */

/*
 * parse_uri - parse URI into filename and CGI args
 *             return 0 if dynamic content, 1 if static

 cgi_key="CGI"
 cgi_root=/home/qqq/
 http://localhost:10000/CGI/abc
 -> /home/qqq/abc
 */


/* $begin parse_uri */
int parse_uri(char *uri, char *filename, char *cgiargs) {
    char *ptr;
    char *cgiptr = strstr(uri, config.cgi_key);

    if (cgiptr == NULL) {  /* Static content */
        strcpy(cgiargs, "");
        strcpy(filename, config.root);
        pathJoin(filename, uri);
        if (uri[strlen(uri) - 1] == '/')
            strcat(filename, "index.html");
        return 1;
    } else {  /* Dynamic content */
        ptr = index(uri, '?');
        if (ptr) {
            strcpy(cgiargs, ptr + 1);
            *ptr = '\0';
        } else
            strcpy(cgiargs, "");
        strcpy(filename, config.cgi_root);
        strcat(filename, cgiptr + strlen(config.cgi_key));
        return 0;
    }
}
/* $end parse_uri */

/*
 * serve_static - copy a file back to the client
 */
/* $begin serve_static */
void serve_static(int fd, char *filename, int filesize, char *method, time_t *tt_request, tm_t *tm_modify) {
    int srcfd;
    char *srcp, filetype[MAXLINE], buf[MAXBUF];
    char modify_time[MAXLINE];
    time_t tt_modify;
    double sec_diff;
    bool need_update;

    /* construct modify time string */
    strftime(modify_time, sizeof(modify_time), time_fmt, tm_modify);

    tt_modify = mktime(tm_modify);
    sec_diff = difftime(*tt_request, tt_modify);
    /* if request >= modify, 说明没有更改
       else request < modify, 说明更改过文件了 */
    need_update = sec_diff < 0;

    /* Send response headers to client */
    get_filetype(filename, filetype);
    if (need_update) {
        sprintf(buf, "HTTP/1.0 200 OK\r\n");
        if (VERBOSE) INFO("[response 200 %s] %s, %s", method, filename, modify_time);
    } else {
        sprintf(buf, "HTTP/1.0 304 OK\r\n");
        if (VERBOSE) INFO("[response 304 %s] %s, %s", method, filename, modify_time);
    }

    sprintf(buf, "%sServer: X Web Server\r\n", buf);
    sprintf(buf, "%sContent-length: %d\r\n", buf, filesize);
    sprintf(buf, "%sLast-Modified: %s\r\n", buf, modify_time);
    sprintf(buf, "%sContent-type: %s\r\n\r\n", buf, filetype);
    Im_rio_writen(fd, buf, strlen(buf));       /* 发送数据给客户端 */



    if (!need_update)
        return;

    if (strcasecmp(method, "HEAD") == 0)
        return;

    if (VERBOSE) INFO("read body");
    /* Send response body to client */
    srcfd = Open(filename, O_RDONLY, 0); /* 打开文件 */
    srcp = Mmap(0, (size_t) filesize, PROT_READ, MAP_PRIVATE, srcfd, 0); /* 映射到一段虚拟空间 */
    Close(srcfd);                           /* 关闭文件 */
    Im_rio_writen(fd, srcp, (size_t) filesize);         /* 发送数据 */
    Munmap(srcp, (size_t) filesize);                 /* 解除映射 */
}

/*
 * get_filetype - derive file type from file name
 */
void get_filetype(char *filename, char *filetype) {
    if (strstr(filename, ".html"))
        strcpy(filetype, "text/html");
    else if (strstr(filename, ".gif"))
        strcpy(filetype, "image/gif");
    else if (strstr(filename, ".jpg"))
        strcpy(filetype, "image/jpeg");
    else if (strstr(filename, ".css"))
        strcpy(filetype, "text/css");
    else if (strstr(filename, ".js"))
        strcpy(filetype, "application/x-javascript");
    else if (strstr(filename, ".png"))
        strcpy(filetype, "image/png");
    else if (strstr(filename, ".mpg") || strstr(filename, ".mp4"))
        strcpy(filetype, "video/mpg");
    else if (strstr(filename, ".svg"))
        strcpy(filetype, "image/svg+xml");
    else if (strstr(filename, ".php"))
        strcpy(filetype, "script/php");

    else if (!strstr(filename, "."))
        strcpy(filetype, "exe");

    else
        strcpy(filetype, "text/plain");
}
/* $end serve_static */

/*
 * serve_dynamic - run a CGI program on behalf of the client
 */
/* $begin serve_dynamic */

void serve_default_dynamic(int fd, char *filename, char *cgiargs, char *method) {

    /* 原始动态处理方法 */
    char buf[MAXLINE], *emptylist[] = {NULL};

    /* Return first part of HTTP response */
    sprintf(buf, "HTTP/1.0 200 OK\r\n");
    Im_rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Server: XServer\r\n");
    Im_rio_writen(fd, buf, strlen(buf));

    if (Fork() == 0) { /* child */
        /* Real server would set all CGI vars here */
        setenv("QUERY_STRING", cgiargs, 1);
        setenv("REQUEST_METHOD", method, 1);
        Dup2(fd, STDOUT_FILENO);         /* Redirect stdout to client */
        /* 默认操作 */
        Signal(SIGPIPE, SIG_DFL);
        Execve(filename, emptylist, environ); /* Run CGI program */
    }
}


void
serve_dynamic(int fd, char *filename, char *cgiargs, unsigned int content_length, char *content_type, char *method) {
    HTTP_HDR req_hdr;
    /* 初始化 */
    req_hdr.req_file = req_hdr.content_type = req_hdr.query_str = req_hdr.req_body = NULL;

    /* 构造请求头 */
    req_hdr.req_file = filename;
    req_hdr.content_type = content_type;

    req_hdr.content_length = content_length;

    if (strcasecmp(method, "GET") == 0) { //GET
        req_hdr.method = HTTP_METHOD_GET;
        req_hdr.query_str = cgiargs;
    } else { // POST
        req_hdr.method = HTTP_METHOD_POST;
        req_hdr.req_body = cgiargs;
    }

//    printf("method: %d\n", req_hdr.method);
//    printf("req file: %s\n", req_hdr.req_file);
//    printf("query str: %s\n", req_hdr.query_str);
//    printf("content type: %s\n", req_hdr.content_type);
//    printf("content length: %u\n", req_hdr.content_length);
//    printf("req body: %s\n", req_hdr.req_body);

    int fpm_fd = open_fpm_sock();               //打开php-fpm socket
    send_fastcgi(&req_hdr, fpm_fd);              //解析请求到FastCGI标准并向socket发送
    recv_fastcgi(fd, fpm_fd);                   //从PHP接受解释完成的输出
    close(fpm_fd);

    /* 原始动态处理方法 */
//    char buf[MAXLINE], *emptylist[] = {NULL};
//
//    /* Return first part of HTTP response */
//    sprintf(buf, "HTTP/1.0 200 OK\r\n");
//    Im_rio_writen(fd, buf, strlen(buf));
//    sprintf(buf, "Server: XServer\r\n");
//    Im_rio_writen(fd, buf, strlen(buf));
//
//    if (Fork() == 0) { /* child */
//        /* Real server would set all CGI vars here */
//        setenv("QUERY_STRING", cgiargs, 1);
//        setenv("REQUEST_METHOD", method, 1);
//        Dup2(fd, STDOUT_FILENO);         /* Redirect stdout to client */
//
//        /* 默认操作 */
//        Signal(SIGPIPE, SIG_DFL);
//
//        Execve(filename, emptylist, environ); /* Run CGI program */
//    }

    // 这里不再需要wait，因为父进程已经监听了子进程的退出事件
//    Wait(NULL); /* Parent waits for and reaps child */
}
/* $end serve_dynamic */

/*
 * clienterror - returns an error message to the client
 */
/* $begin clienterror */
void clienterror(int fd, char *cause, char *errnum,
                 char *shortmsg, char *longmsg) {
    char buf[MAXLINE], body[MAXBUF];

    /* Build the HTTP response body */
    sprintf(body, "<html><title>Xserver Error</title>");
    sprintf(body, "%s<body bgcolor=""ffffff"">\r\n", body);
    sprintf(body, "%s%s: %s\r\n", body, errnum, shortmsg);
    sprintf(body, "%s<p>%s: %s\r\n", body, longmsg, cause);
    sprintf(body, "%s<hr><em>The X Web server</em>\r\n", body);

    /* Print the HTTP response */
    sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
    WARN("[HTTP/1.0 %s %s] %s: %s", errnum, shortmsg, longmsg, cause);
    Im_rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Content-type: text/html\r\n");
    Im_rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Content-length: %d\r\n\r\n", (int) strlen(body));
    Im_rio_writen(fd, buf, strlen(buf));
    Im_rio_writen(fd, body, strlen(body));
}

/* $end clienterror */