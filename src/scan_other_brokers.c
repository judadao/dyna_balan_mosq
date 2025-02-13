// hello_thread.c
#include "scan_other_brokers.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <fcntl.h>
#include <errno.h> 


#define MQTT_CONNECT 0x10   // MQTT CONNECT 請求的消息類型
#define MQTT_CONACK 0x20    // MQTT CONNACK 回應的消息類型

#define START_PORT 1883
#define END_PORT 1885
#define LOCALHOST "127.0.0.1"
#define TIMEOUT_SECONDS 10  

// 這是執行緒運行的函數，每 10 秒鐘打印一次 "Hello"
void *scan_loop(void *arg) {
    const char *ip = LOCALHOST; // 使用 127.0.0.1 作為 IP 地址
    printf("\r\n Scanning ports from %d to %d on %s...\n", START_PORT, END_PORT, ip);
    while (1) {
        // scan_ports(ip);
        sleep(10);  // 每 10 秒鐘打印一次 "Hello"
    }
    return NULL;
}


// 發送 MQTT CONNECT 請求並接收回應
int check_mqtt_broker(const char *ip, int port) {
    struct sockaddr_in addr;
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    
    if (sockfd < 0) {
        perror("Socket creation failed");
        return -1;
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(ip);

    // 設置超時
    struct timeval timeout;
    timeout.tv_sec = TIMEOUT_SECONDS;  // 設置超時為 2 秒
    timeout.tv_usec = 0;

    // 設置 socket 為非阻塞模式
    fcntl(sockfd, F_SETFL, O_NONBLOCK);

    // 嘗試連接
    int res = connect(sockfd, (struct sockaddr *)&addr, sizeof(addr));

    if (res < 0) {
        fd_set fdset;
        FD_ZERO(&fdset);
        FD_SET(sockfd, &fdset);

        // 使用 select 等待直到超時
        res = select(sockfd + 1, NULL, &fdset, NULL, &timeout);
        
        if (res == 0) {
            // 超時
            close(sockfd);
            return 0;  // 連接超時
        } else if (res < 0) {
            perror("Select failed");
            close(sockfd);
            return -1;  // 發生錯誤
        }
    }

    // 構建 MQTT CONNECT 請求
    unsigned char connect_packet[] = {
        0x10, 0x0C, 0x00, 0x04, 'M', 'Q', 'T', 'T', // 協議名稱（MQTT）
        0x04, // 協議級別（4）
        0x02, // 標誌位，表示 clean session
        0x00, 0x3C, // Keep alive 設置為 60 秒
    };

    // 發送 CONNECT 請求
    ssize_t sent = send(sockfd, connect_packet, sizeof(connect_packet), 0);
    if (sent < 0) {
        perror("Send failed");
        close(sockfd);
        return 0;  // 發送失敗
    }else{
        printf("Send success\n");
    }
    // 接收 CONNACK 回應
    unsigned char connack_packet[4];
    ssize_t received = recv(sockfd, connack_packet, sizeof(connack_packet), 0);
    if (received < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            // 收到 "Resource temporarily unavailable" 錯誤，表示套接字緩衝區暫時不可用
            printf("recv failed with temporary unavailability\n");
        } else {
            perror("Recv failed");
        }
        close(sockfd);
        return 0;  // recv 失敗
    }

    // 檢查是否收到 CONNACK 回應
    printf("\r\n connack: %d\n", connack_packet[0]);
    if (connack_packet[0] == MQTT_CONACK && connack_packet[3] == 0) {
        close(sockfd);
        return 1; // 如果是 CONNACK 且回應碼為 0，則是 MQTT Broker
    }

    close(sockfd);
    return 0; // 如果不是有效的 CONNACK 回應，則不是 MQTT Broker
}

// 掃描端口範圍來檢查是否有 MQTT Broker
void scan_ports(const char *ip) {
    for (int port = START_PORT; port <= END_PORT; port++) {
        if (check_mqtt_broker(ip, port)) {
            printf("Found MQTT Broker at %s:%d\n", ip, port);
        } else {
            printf("### No MQTT Broker at %s:%d ####\n", ip, port);
        }
        printf("\r\n");
    }
}
// 啟動執行緒的函數
void scan_other_brokers_thread() {
    pthread_t scan_brokers_thread;  // 執行緒的標識符

    // 創建並啟動新的執行緒
    int rc = pthread_create(&scan_brokers_thread, NULL, scan_loop, NULL);
    if (rc) {
        printf("Error creating thread: %d\n", rc);
        return;
    }

    // 等待執行緒結束（如果有結束的話）
    // pthread_join(hello_thread, NULL);
}
