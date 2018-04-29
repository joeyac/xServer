# for network design
## implement a multi threading web server in HTTP1.0 or higher

1. understand the tiny web in [Computer Systems A Programmer's perspective]()

2. using multi threading or IO/multi

3. using queue and Producer consumer model


### Feature

1. 增加了对post方法的支持(1.1)
2. 增加了对静态文件版本号识别的支持
3. 增加对中文路径的解码支持，主要是改写php中的url decode函数
4. 重写了路径拼接函数
5. 简单使用多进程处理并发
6. 增加对head方法的支持：
    >from rfc2626 section 9.4 HEAD
     The HEAD method is identical to GET except that the server
     MUST NOT return a message-body in the response.
     
    >利用telnet来发head请求：
    HEAD / HTTP/1.0