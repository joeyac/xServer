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
    
### TODO LIST

1. 缓存支持，Last-Modified和ETag header设置
2. 
    
### 版本注记

1. 1.0版本为csapp初始版本加一些函数的友好封装，日志记录等
2. 1.1版本增加了对post方法的支持
3. 1.2版本增加了对http 1.0 的基本支持
4. 1.3版本增加了多进程并发支持，修复了内存泄露，回收了僵尸进程
5. TODO: 1.4版本增加Last-Modified/If-Modified-Since的发送和检验，支持浏览器缓存