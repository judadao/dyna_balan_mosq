// scan_other_brokers.h
#ifndef SCAN_OTHER_BROKERS_THREAD_H
#define SCAN_OTHER_BROKERS_THREAD_H

#include <pthread.h>

// 宣告執行緒函數
void *scan_loop(void *arg);  // 每 10 秒鐘打印 "Hello" 的執行緒函數
void scan_ports(const char *ip);
void scan_other_brokers_thread();     // 啟動執行緒的函數

#endif // SCAN_OTHER_BROKERS_THREAD_H
