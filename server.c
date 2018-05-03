//
// Created by xjw on 4/27/18.
//

#include "server.h"
#include "utils.h"

const char *time_fmt = "%a, %d %b %Y %T %Z";


/* improved rio written, handle EPIPE signal */
void Im_rio_writen(int fd, void *usrbuf, size_t n) {
    if (rio_writen(fd, usrbuf, n) != n) {
        if (errno == EPIPE) {
            WARN("%s: connection ended by customer.", strerror(errno));
            exit(0);
        }
        unix_error("Rio_writen error");
    }
}

/*
 * doit - handle one HTTP request/response transaction
 * 对静态文件有get和head方法
 * 对动态文件有get和post和head方法
 */
/* $begin doit */
void doit(int fd)
{
    int is_static;      /* 是否为静态文件 */
    struct stat sbuf;  /* 用于获得文件的信息 */
    char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
    char filename[MAXLINE], cgiargs[MAXLINE];
    struct tm tm_request, tm_modify;
    time_t tt_request = 0;
    bool tm_done;

    rio_t rio;

    /* Read request line and headers */
    Rio_readinitb(&rio, fd);    /* 对rio进行初始化 */
    Rio_readlineb(&rio, buf, MAXLINE);       /* 读取一行数据 */
    if (strlen(buf) == 0) {
        if (VERBOSE) WARN("empty request header");
        return;
    }

    sscanf(buf, "%s %s %s", method, uri, version);
    INFO("[%s %s] %s", version, method, uri);

    /* strcasecmp 忽略大小写比较字符串， 相等返回0 */
    if (!(strcasecmp(method, "GET") == 0 || strcasecmp(method, "POST") == 0 || strcasecmp(method, "HEAD") == 0)) {
        clienterror(fd, method, "501", "Not Implemented",
                    "XServer does not implement this method"); /* 暂时只支持GET POST 方法 */
        return;
    }
    int param_len = read_requesthdrs(&rio, method, &tm_request, &tm_done);
    if (tm_done) tt_request = mktime(&tm_request);  // 如果有If-Modified-Since字段

    if (param_len) Rio_readnb(&rio, buf, (size_t) param_len);

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
    }
    else { /* Serve dynamic content */
        if (!(S_ISREG(sbuf.st_mode)) || !(S_IXUSR & sbuf.st_mode)) {
            clienterror(fd, filename, "403", "Forbidden",
                        "xServer couldn't run the CGI program");
            return;
        }
        if (strcasecmp(method, "GET") == 0)
            serve_dynamic(fd, filename, cgiargs, method);
        else
            serve_dynamic(fd, filename, buf, method);
    }
}
/* $end doit */

/*
 * read_requesthdrs - read and parse HTTP request headers
 */
/* $begin read_requesthdrs */
int read_requesthdrs(rio_t *rp, char *method, tm_t *ti, bool *tm_done)
{
    char buf[MAXLINE];
    char timeStr[MAXLINE];
    int len = 0;
    do {
        Rio_readlineb(rp, buf, MAXLINE);
        if (VERBOSE) INFO("%s", buf);
        if (strncasecmp(buf, "If-Modified-Since:", 18) == 0) { // 判断是否有If-Modified-Since字段
            sscanf(buf, "If-Modified-Since: %[^\n]", timeStr);
            strptime(timeStr, time_fmt, ti);
            *tm_done = true;
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
 */
/* $begin parse_uri */
int parse_uri(char *uri, char *filename, char *cgiargs)
{
    char *ptr;

    if (!strstr(uri, "cgi-bin")) {  /* Static content */
        strcpy(cgiargs, "");
        strcpy(filename, config.root);
        pathJoin(filename, uri);
        if (uri[strlen(uri)-1] == '/')
            strcat(filename, "index.html");
        return 1;
    }
    else {  /* Dynamic content */
        ptr = index(uri, '?');
        if (ptr) {
            strcpy(cgiargs, ptr+1);
            *ptr = '\0';
        }
        else
            strcpy(cgiargs, "");
        strcpy(filename, config.root);
        strcat(filename, uri);
        return 0;
    }
}
/* $end parse_uri */

/*
 * serve_static - copy a file back to the client
 */
/* $begin serve_static */
void serve_static(int fd, char *filename, int filesize, char *method, time_t *tt_request, tm_t *tm_modify)
{
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
        INFO("response 200: %s: %s, %s", method, filename, modify_time);
    } else {
        sprintf(buf, "HTTP/1.0 304 OK\r\n");
        INFO("response 304: %s: %s, %s", method, filename, modify_time);
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
void get_filetype(char *filename, char *filetype)
{
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
    else if(strstr(filename, ".mpg") || strstr(filename, ".mp4"))
        strcpy(filetype, "video/mpg");
    else
        strcpy(filetype, "text/plain");
}
/* $end serve_static */

/*
 * serve_dynamic - run a CGI program on behalf of the client
 */
/* $begin serve_dynamic */
void serve_dynamic(int fd, char *filename, char *cgiargs, char *method)
{
    char buf[MAXLINE], *emptylist[] = { NULL };

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
    Wait(NULL); /* Parent waits for and reaps child */
}
/* $end serve_dynamic */

/*
 * clienterror - returns an error message to the client
 */
/* $begin clienterror */
void clienterror(int fd, char *cause, char *errnum,
                 char *shortmsg, char *longmsg)
{
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
    sprintf(buf, "Content-length: %d\r\n\r\n", (int)strlen(body));
    Im_rio_writen(fd, buf, strlen(buf));
    Im_rio_writen(fd, body, strlen(body));
}

/* $end clienterror */