//
// Created by xjw on 4/28/18.
//
#include "../utils.h"
#include <hiredis/hiredis.h>

void *monitor(void *vargp) {
    Pthread_detach(Pthread_self());
    struct timeval timeout = {2, 0};    //2s的超时时间
    redisContext *client;
    redisReply *pRedisReply;
    client = redisConnectWithTimeout("127.0.0.1", 6379, timeout);
    while (1) {
        pRedisReply = (redisReply*)redisCommand(client, "LLEN %s", "queue");
        INFO("length: %lld\n", pRedisReply->integer);
        freeReplyObject(pRedisReply);
        Sleep(1);
    }
}

void *producer(void *vargp) {
    Pthread_detach(Pthread_self());
    struct timeval timeout = {2, 0};    //2s的超时时间
    redisContext *client;
    redisReply *pRedisReply;
    client = redisConnectWithTimeout("127.0.0.1", 6379, timeout);
    int i = 0;
    while (1) {
        pRedisReply = (redisReply*)redisCommand(client, "RPUSH %s %d", "queue", i);
        INFO("insert: %d\n", i);
        freeReplyObject(pRedisReply);
        Sleep(1);
        i = i + 1;
    }
}

void *consumer(void *vargp) {
    Pthread_detach(Pthread_self());
    int id = *(int*)vargp;
    struct timeval timeout = {2, 0};    //2s的超时时间
    redisContext *client;
    redisReply *pRedisReply;
    client = redisConnectWithTimeout("127.0.0.1", 6379, timeout);
    while (1) {
        pRedisReply = (redisReply*)redisCommand(client, "BLPOP %s 1", "queue");
        if (pRedisReply->type == REDIS_REPLY_NIL) {
            WARN("empty reply: %s\n", pRedisReply->str);
            //client = redisConnectWithTimeout("127.0.0.1", 6379, timeout);
        } else {
            INFO("Redis(%d): %s => %s\n", id, pRedisReply->element[0]->str, pRedisReply->element[1]->str);
        }
        freeReplyObject(pRedisReply);
        sleep(2);
    }
}

const int MAX_THREADS = 8;
const int MAX_QUEUE = 5;

int main() {
    initLogger();
    pthread_t t[MAX_THREADS + 1];
//    Pthread_create(&t1, NULL, monitor, NULL);
//    Pthread_create(&t1, NULL, producer, NULL);
//    Pthread_create(&t2, NULL, consumer, NULL);

    int cur = 1;
    Pthread_create(&t[0], NULL, producer, NULL);
    sleep(10);
    int i = 1;
    Pthread_create(&t[1], NULL, consumer, &i);
    struct timeval timeout = {2, 0};    //2s的超时时间
    redisContext *client;
    redisReply *pRedisReply;
    client = redisConnectWithTimeout("127.0.0.1", 6379, timeout);
    while (1) {
        pRedisReply = (redisReply*)redisCommand(client, "LLEN %s", "queue");
        if (pRedisReply->integer >= MAX_QUEUE && cur < MAX_THREADS) {
            for (i = cur + 1; i <= 2 * cur; i++) {
                Pthread_create(&t[i], NULL, consumer, &i);
            }
            cur <<= 1;
            INFO("cur: %d\n", cur);
        } else if (pRedisReply->integer == 0 && cur > 1) {
            for (i = cur; i > cur / 2; i--) {
                Pthread_cancel(t[i]);
            }
            cur >>= 1;
            INFO("cur: %d\n", cur);
        }
        freeReplyObject(pRedisReply);
        sleep(1);
    }
    exit(0);
}