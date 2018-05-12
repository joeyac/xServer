//
// Created by ruofeng on 18-5-6.
//

#include "fcgi.h"

int open_fpm_sock() {
    int sock;
    struct sockaddr_in serv_addr;

    // 创建套接字
    sock = socket(PF_INET, SOCK_STREAM, 0);
    if (-1 == sock) {
        fprintf(stderr, "socket error");
        return -1;
    }

    // 设定地址
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(PHP_FPM_HOST);
    serv_addr.sin_port = htons(PHP_FPM_PORT);

    // 连接php-fpm
    if (-1 == connect(sock, (struct sockaddr *) &serv_addr, sizeof(serv_addr))) {
        fprintf(stderr, "connect error");
        return -1;
    }

    return sock;
}

FCGI_Header makeHeader(
        int type,
        int requestId,
        size_t contentLength,
        size_t paddingLength) {
    FCGI_Header header;
    header.version = FCGI_VERSION_1;
    header.type = (unsigned char) type;
    header.requestIdB1 = (unsigned char) ((requestId >> 8) & 0xff);
    header.requestIdB0 = (unsigned char) ((requestId) & 0xff);
    header.contentLengthB1 = (unsigned char) ((contentLength >> 8) & 0xff);
    header.contentLengthB0 = (unsigned char) ((contentLength) & 0xff);
    header.paddingLength = (unsigned char) paddingLength;
    header.reserved = 0;
    return header;
}

FCGI_BeginRequestBody makeBeginRequestBody(int role, int keepConn) {
    FCGI_BeginRequestBody body;
    body.roleB1 = (unsigned char) ((role >> 8) & 0xff);
    body.roleB0 = (unsigned char) (role & 0xff);
    body.flags = (unsigned char) ((keepConn) ? 1 : 0); // 1为长连接，0为短连接
    memset(body.reserved, 0, sizeof(body.reserved));
    return body;
}

int sendBeginRequestRecord(int fd, int requestId) {
    int ret;

    FCGI_BeginRequestRecord beginRecord;

    beginRecord.header = makeHeader(FCGI_BEGIN_REQUEST, requestId, sizeof(beginRecord.body), 0);
    beginRecord.body = makeBeginRequestBody(FCGI_RESPONDER, 0);

    ret = (int) rio_writen(fd, &beginRecord, sizeof(beginRecord));

    if (ret == sizeof(beginRecord))
        return 0;
    else
        return -1;
}

int sendEndRequestRecord(int fd, int requestId) {
    int ret;

    FCGI_Header endHeader;

    endHeader = makeHeader(FCGI_END_REQUEST, requestId, 0, 0);

    ret = (int) rio_writen(fd, &endHeader, FCGI_HEADER_LEN);

    if (ret == FCGI_HEADER_LEN)
        return 0;
    else
        return -1;
}

int makeNameValueBody(char *name, size_t nameLen, char *value, size_t valueLen,
                      unsigned char *bodyBuffPtr, size_t *bodyLenPtr) {
    unsigned char *startBodyBuffPtr = bodyBuffPtr;  //记录body的开始位置

    if (nameLen < 128)//如果nameLen长度小于128字节
    {
        *bodyBuffPtr++ = (unsigned char) nameLen;
    } else {
        //nameLen用4个字节保存
        *bodyBuffPtr++ = (unsigned char) ((nameLen >> 24) | 0x80);
        *bodyBuffPtr++ = (unsigned char) (nameLen >> 16);
        *bodyBuffPtr++ = (unsigned char) (nameLen >> 8);
        *bodyBuffPtr++ = (unsigned char) nameLen;
    }

    if (valueLen < 128)  //valueLen小于128就用一个字节保存
    {
        *bodyBuffPtr++ = (unsigned char) valueLen;
    } else {
        //valueLen用4个字节保存
        *bodyBuffPtr++ = (unsigned char) ((valueLen >> 24) | 0x80);
        *bodyBuffPtr++ = (unsigned char) (valueLen >> 16);
        *bodyBuffPtr++ = (unsigned char) (valueLen >> 8);
        *bodyBuffPtr++ = (unsigned char) valueLen;
    }
    memcpy(bodyBuffPtr, name, nameLen);
    bodyBuffPtr += nameLen;
    memcpy(bodyBuffPtr, value, valueLen);
    bodyBuffPtr += valueLen;

    //计算出body的长度
    *bodyLenPtr = bodyBuffPtr - startBodyBuffPtr;

    return 1;
}

int sendParamsRecord(int fd, char *name, char *value) {
    int requestId = fd;
    unsigned char bodyBuff[1024];

    bzero(bodyBuff, sizeof(bodyBuff));

    size_t bodyLen;//保存body的长度

    //生成PARAMS参数内容的body
    makeNameValueBody(name, strlen(name), value, strlen(value), bodyBuff, &bodyLen);

    FCGI_Header nameValueHeader;
    nameValueHeader = makeHeader(FCGI_PARAMS, requestId, bodyLen, 0);

    size_t nameValueRecordLen = bodyLen + FCGI_HEADER_LEN;
    char nameValueRecord[nameValueRecordLen];

    //将头和body拷贝到一块buffer中只需调用一次write
    memcpy(nameValueRecord, (char *) &nameValueHeader, FCGI_HEADER_LEN);
    memcpy(nameValueRecord + FCGI_HEADER_LEN, bodyBuff, bodyLen);

    ssize_t ret = rio_writen(fd, nameValueRecord, nameValueRecordLen);
    if (ret == nameValueRecordLen)
        return 1;
    else
        return -1;
}

int sendEmptyParamsRecord(int fd) {
    int requestId = fd;
    size_t ret;
    FCGI_Header nvHeader = makeHeader(FCGI_PARAMS, requestId, 0, 0);
    ret = (size_t) rio_writen(fd, (char *) &nvHeader, FCGI_HEADER_LEN);

    if (ret == FCGI_HEADER_LEN) {
        return 0;
    } else {
        return -1;
    }
}

int sendStdinRecord(int fd, char *data, int len) {
    int requestId = fd;
    size_t cl = (size_t) len, pl, ret;
    char buf[8] = {0};

    while (len > 0) {
        // 判断STDIN数据是否大于传输最大值FCGI_MAX_LENGTH
        if (len > FCGI_MAX_LENGTH) {
            cl = FCGI_MAX_LENGTH;
        }

        // 计算填充数据长度
        pl = (cl % 8) == 0 ? 0 : 8 - (cl % 8);

        FCGI_Header sinHeader = makeHeader(FCGI_STDIN, requestId, cl, pl);
        ret = (size_t) rio_writen(fd, (char *) &sinHeader, FCGI_HEADER_LEN);  // 发送协议头部
        if (ret != FCGI_HEADER_LEN) {
            return -1;
        }

        ret = (size_t) rio_writen(fd, data, cl); // 发送stdin数据
        if (ret != cl) {
            return -1;
        }

        if (pl > 0) {
            ret = (size_t) rio_writen(fd, buf, pl); // 发送填充数据
            if (ret != pl) {
                return -1;
            }
        }

        len -= cl;
        data += cl;
    }

    return 0;
}

int sendEmptyStdinRecord(int fd) {
    int requestId = fd;
    size_t ret;
    FCGI_Header sinHeader = makeHeader(FCGI_STDIN, requestId, 0, 0);
    ret = (size_t) rio_writen(fd, (char *) &sinHeader, FCGI_HEADER_LEN);

    if (ret == FCGI_HEADER_LEN) {
        return 0;
    } else {
        return -1;
    }
}

int send_fastcgi(struct http_req_hdr *hdr, int sock) {
    int requestId = sock;
    if (sendBeginRequestRecord(sock, requestId) < 0) {
        fprintf(stderr, "sendBeginRequestRecord error");
        return -1;
    }

    char *n1 = "SCRIPT_FILENAME";
    sendParamsRecord(sock, n1, hdr->req_file);

    char *n2 = "REQUEST_METHOD";
    char *v2;
    switch (hdr->method) {
        case HTTP_METHOD_HEAD:
            v2 = "HEAD";
            break;
        case HTTP_METHOD_GET:
            v2 = "GET";
            break;
        case HTTP_METHOD_POST:
            v2 = "POST";
            break;
        default:
            v2 = "GET";
    }
    sendParamsRecord(sock, n2, v2);

    if (hdr->query_str != NULL) {
        sendParamsRecord(sock, "QUERY_STRING", hdr->query_str);
    }

    if (hdr->method == HTTP_METHOD_POST) {
        char conlen[20];
        sprintf(conlen, "%d", hdr->content_length);
        sendParamsRecord(sock, "HTTP_CONTENT_TYPE", hdr->content_type);
        sendParamsRecord(sock, "HTTP_CONTENT_LENGTH", conlen);
        sendParamsRecord(sock, "CONTENT_TYPE", hdr->content_type);
        sendParamsRecord(sock, "CONTENT_LENGTH", conlen);
    }

    //发送空的Params表示属性传递结束
    sendEmptyParamsRecord(sock);

    //发送POST数据 STDIN
    if (hdr->method == HTTP_METHOD_POST) {
        sendStdinRecord(sock, hdr->req_body, hdr->content_length);
        sendEmptyStdinRecord(sock);
    }

    //发送结束Record
    if (sendEndRequestRecord(sock, requestId) < 0) {
        fprintf(stderr, "sendEndRequestRecord error");
        return -1;
    }
    return 0;
}

int recv_fastcgi(int cfd, int fd) {
    int requestId = fd;

    FCGI_Header responHeader;
    FCGI_EndRequestBody endr;
    char *conBuf = NULL, *errBuf = NULL;
    int buf[8];
    size_t cl;
    ssize_t ret;
    int fcgi_rid;  // 保存fpm发送过来的request id

    size_t outlen = 0, errlen = 0, lastlen = 0;

    // 读取协议记录头部
    while (rio_readn(fd, &responHeader, FCGI_HEADER_LEN) > 0) {
        lastlen = outlen;
        fcgi_rid = (int) (responHeader.requestIdB1 << 8) + (int) (responHeader.requestIdB0);
        if (responHeader.type == FCGI_STDOUT && fcgi_rid == requestId) {
            // 获取内容长度
            cl = (responHeader.contentLengthB1 << 8) + (responHeader.contentLengthB0);
            outlen += cl;

            // 如果不是第一次读取FCGI_STDOUT记录
            if (conBuf != NULL) {
                //free(conBuf);
                //conBuf=malloc(outlen);
                //FIXME: 分两次输出内容时会出错: realloc(): invalid next size
                conBuf = realloc(conBuf, outlen);
            } else {
                conBuf = (char *) malloc(cl);
            }
            //ret=read(fd,conBuf,cl);
            ret = rio_readn(fd, conBuf + lastlen, cl);    //从fpm中读取cl长度数据到conBuf中，返回的是读取的数据量
            if (ret == -1 || ret != cl) {       //返回值异常，读取出错
                printf("read fcgi_stdout record error\n");
                return -1;
            }

            // 读取填充内容，忽略
            if (responHeader.paddingLength > 0) {
                ret = rio_readn(fd, buf, responHeader.paddingLength);
                if (ret == -1 || ret != responHeader.paddingLength) {
                    printf("read fcgi_stdout padding error %d\n", responHeader.paddingLength);
                    return -1;
                }
            }
        } else if (responHeader.type == FCGI_STDERR && fcgi_rid == requestId) {
            cl = (responHeader.contentLengthB1 << 8) + (responHeader.contentLengthB0);
            errlen += cl;
            if (errBuf != NULL)
                errBuf = realloc(errBuf, errlen);
            else
                errBuf = (char *) malloc(cl);
            ret = rio_readn(fd, errBuf, cl);
            if (ret == -1 || ret != cl)
                return -1;
            if (responHeader.paddingLength > 0) {
                ret = rio_readn(fd, buf, responHeader.paddingLength);
                if (ret == -1 || ret != responHeader.paddingLength)
                    return -1;
            }
        } else if (responHeader.type == FCGI_END_REQUEST && fcgi_rid == requestId) {
            ret = rio_readn(fd, &endr, sizeof(FCGI_EndRequestBody));
            if (ret == -1 || ret != sizeof(FCGI_EndRequestBody)) {
                free(conBuf);
                free(errBuf);
                return -1;
            }
            send_to_client(cfd, outlen, conBuf, errlen, errBuf);    //将buf内容输出到客户端socket
            free(conBuf);   //FIXME: PHP输出内容过长，但是仍然只是一次TCP传输的数据在这里free会出错free(): invalid next size (normal)
            free(errBuf);
            return 0;
        }
    }
    return 0;
};

ssize_t send_to_client(int fd, size_t outlen, char *out, size_t errlen, char *err) {
    char *p;
    int n;
    char buf[BUFSIZ];
    //200 "Content-type: text/html; charset=UTF-8\r\n\r\nhello from phper/dist"
    //302 "Status: 302 Found\r\nLocation: /\r\nContent-type: text/html; charset=UTF-8\r\n\r\n"
    //500 "Status: 500 Internal Server Error\r\nContent-type: text/html; charset=UTF-8\r\n\r\n"
    char *http_res_tmpl = "HTTP/1.1 %s %s\r\n"
                          "Server: ruofeng's Server\r\n"
                          "Content-Length: %d\r\n"
                          "Content-Type: %s\r\n\r\n";

    if (strncmp(out, "Status:", 7) == 0) {
        char status[3];             //保存状态码
        char status_str[30];        //保存状态说明字符串
        char response_type[50];     //保存内容类型
        char *line = strsep(&out, "\r\n");
        out++;
        while (line != NULL) {
            //直接跳出不再解析响应头部
            if (strncmp(line, "\0", 1) == 0)
                break;
            //解析状态码与状态说明字符串
            if (strncmp(line, "Status:", 7) == 0) {
                strncpy(status, line + 8, 3);
                strcpy(status_str, line + 12);
            }
            if (strncmp(line, "Content-type:", 13) == 0) {
                strcpy(response_type, line + 14);
            }
            line = strsep(&out, "\r\n");
            out++;
        }
        sprintf(buf, http_res_tmpl, status, status_str, errlen, response_type);
        rio_writen(fd, buf, strlen(buf));
        rio_writen(fd, err, errlen);
        return errlen;
    }

    //开头未输出状态码则默认为200
    p = index(out, '\r');
    n = (int) (p - out);
    sprintf(buf, http_res_tmpl, "200", "OK", outlen - n - 4, "text/html");
    rio_writen(fd, buf, strlen(buf));
    rio_writen(fd, p + 4, outlen - n - 4);
    return outlen;
}