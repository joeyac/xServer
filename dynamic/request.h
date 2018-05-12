//
// Created by xjw on 5/12/18.
//

#ifndef XSERVER_REQUEST_H
#define XSERVER_REQUEST_H

#define HTTP_METHOD_UNKNOWN         0x0001
#define HTTP_METHOD_GET             0x0002
#define HTTP_METHOD_HEAD            0x0004
#define HTTP_METHOD_POST            0x0008

typedef struct http_req_hdr {
    int method;
    char *host;
    char *uri;
    char *accept_type;
    char *req_file;     //请求文件完整路径
    char *query_str;    //请求query string，不带问号
    char *content_type;
    unsigned int content_length;
    char *req_body;

} HTTP_HDR, *PHTTP_HDR;

#endif //XSERVER_REQUEST_H


