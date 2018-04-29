//
// Created by xjw on 4/27/18.
//

#include "server.h"
#include "utils.h"

/*
 * doit - handle one HTTP request/response transaction
 */
/* $begin doit */
void doit(int fd)
{
    int is_static;      /* 是否为静态文件 */
    struct stat sbuf;  /* 用于获得文件的信息 */
    char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
    char filename[MAXLINE], cgiargs[MAXLINE];
    rio_t rio;

    /* Read request line and headers */
    Rio_readinitb(&rio, fd);    /* 对rio进行初始化 */
    Rio_readlineb(&rio, buf, MAXLINE);       /* 读取一行数据 */
    sscanf(buf, "%s %s %s", method, uri, version);
    INFO("%s", buf);

    if (!(strcasecmp(method, "GET") == 0 || strcasecmp(method, "POST") == 0)) { /* strcasecmp 忽略大小写比较字符串， 相等返回0 */
        clienterror(fd, method, "501", "Not Implemented",
                    "XServer does not implement this method"); /* 暂时只支持GET POST 方法 */
        return;
    }
    int param_len = read_requesthdrs(&rio, method);

    Rio_readnb(&rio, buf, (size_t) param_len);

    /* Parse URI from GET request */
    is_static = parse_uri(uri, filename, cgiargs);
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
        serve_static(fd, filename, (int) sbuf.st_size);
    }
    else { /* Serve dynamic content */
        if (!(S_ISREG(sbuf.st_mode)) || !(S_IXUSR & sbuf.st_mode)) {
            clienterror(fd, filename, "403", "Forbidden",
                        "xServer couldn't run the CGI program");
            return;
        }
        if (strcasecmp(method, "GET") == 0)
            serve_dynamic(fd, filename, cgiargs);
        else
            serve_dynamic(fd, filename, buf);
    }
}
/* $end doit */

/*
 * read_requesthdrs - read and parse HTTP request headers
 */
/* $begin read_requesthdrs */
int read_requesthdrs(rio_t *rp, char *method)
{
    char buf[MAXLINE];
    int len = 0;

    do {
        Rio_readlineb(rp, buf, MAXLINE);
        INFO("%s", buf);
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
        strcat(filename, uri);
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
void serve_static(int fd, char *filename, int filesize)
{
    int srcfd;
    char *srcp, filetype[MAXLINE], buf[MAXBUF];

    /* Send response headers to client */
    get_filetype(filename, filetype);
    sprintf(buf, "HTTP/1.0 200 OK\r\n");
    sprintf(buf, "%sServer: X Web Server\r\n", buf);
    sprintf(buf, "%sContent-length: %d\r\n", buf, filesize);
    sprintf(buf, "%sContent-type: %s\r\n\r\n", buf, filetype);
    Rio_writen(fd, buf, strlen(buf));       /* 发送数据给客户端 */

    /* Send response body to client */
    srcfd = Open(filename, O_RDONLY, 0); /* 打开文件 */
    srcp = Mmap(0, (size_t) filesize, PROT_READ, MAP_PRIVATE, srcfd, 0); /* 映射到一段虚拟空间 */
    Close(srcfd);                           /* 关闭文件 */
    Rio_writen(fd, srcp, (size_t) filesize);         /* 发送数据 */
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
void serve_dynamic(int fd, char *filename, char *cgiargs)
{
    char buf[MAXLINE], *emptylist[] = { NULL };

    /* Return first part of HTTP response */
    sprintf(buf, "HTTP/1.0 200 OK\r\n");
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Server: XServer\r\n");
    Rio_writen(fd, buf, strlen(buf));

    if (Fork() == 0) { /* child */
        /* Real server would set all CGI vars here */
        setenv("QUERY_STRING", cgiargs, 1);
        Dup2(fd, STDOUT_FILENO);         /* Redirect stdout to client */
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
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Content-type: text/html\r\n");
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Content-length: %d\r\n\r\n", (int)strlen(body));
    Rio_writen(fd, buf, strlen(buf));
    Rio_writen(fd, body, strlen(body));
}
/* $end clienterror */