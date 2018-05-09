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
    
    ps -mp 12659
    
    lsof -g 12659
    
### TODO LIST

1. 缓存支持，Last-Modified和ETag header设置
2. 
    
### 版本注记

1. 1.0版本为csapp初始版本加一些函数的友好封装，日志记录等
2. 1.1版本增加了对post方法的支持
3. 1.2版本增加了对http 1.0 的基本支持
4. 1.3版本增加了多进程并发支持，修复了内存泄露，回收了僵尸进程
5. 1.4版本增加Last-Modified/If-Modified-Since的发送和检验，支持浏览器缓存, 动态目录和url映射
6. 2.0版本使用多线程，简单修改了多进程版本，每来一个请求建立一个线程
7. 2.1版本使用简单线程池，生产者消费者模型, 修复了socket泄露，
~~当队列中数据个数大于队列长度一半时增加线程数量到上界，否则减少线程数量到下界~~
动态调整作用不大，消耗资源并且不现实，事实只需要用队列锁即可
8. 2.2版本使用 ~~多进程~~ +线程池,每个进程绑定到一个cpu核心



## something else
    想用hiredis的异步命令，发现有一些坑= = 
    
    hiredis那个github页面很多东西没有写清楚，如果要使用redis客户端的异步通信，单靠hiredis自带的那几个api是不够的，还需要事件触发库的支持。
    
    我选择了事件触发库libevent http://libevent.org/
    
    首先去官网上下载需要的libevent库
    
    安装命令：
    
    ```
    ./configure
    sudo make
    sudo make install
    sudo ln -s /usr/local/lib/libevent-2.1.so.6 /usr/lib/libevent-2.1.so.6
    ```
    
    https://www.ibm.com/developerworks/cn/aix/library/au-libev/index.html



## 压力测试命令

    wrk -d5 http://127.0.0.1:10000/
    
    ab -n 10000 -c 1000 http://127.0.0.1:10000/
