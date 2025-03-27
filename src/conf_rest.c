#include <microhttpd.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/select.h>

static int handle_request(void *cls, struct MHD_Connection *connection, const char *url, const char *method,
                          const char *version, const char *upload_data, size_t *upload_data_size, void **con_cls) {
    struct MHD_Response *response;
    int ret;

    // 打印请求的 URL 和方法
    printf("Received %s request for %s\n", method, url);

    // 处理 GET 请求
    if (strcmp(method, "GET") == 0) {
        const char *response_text = "Hello, world! This is a simple HTTP server!";
        
        // 创建一个响应
        response = MHD_create_response_from_buffer(strlen(response_text), (void*)response_text, MHD_RESPMEM_PERSISTENT);
        
        // 返回 200 OK 的响应
        ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
        MHD_destroy_response(response);
        return ret;
    }

    // 默认返回 405 Method Not Allowed 错误
    const char *response_text = "Method Not Allowed";
    response = MHD_create_response_from_buffer(strlen(response_text), (void*)response_text, MHD_RESPMEM_PERSISTENT);
    ret = MHD_queue_response(connection, MHD_HTTP_METHOD_NOT_ALLOWED, response);
    MHD_destroy_response(response);
    return ret;
}

// 启动 HTTP 服务器
int start_http_server() {
    struct MHD_Daemon *daemon;

    // 启动 HTTP 服务器，监听端口 8080
    daemon = MHD_start_daemon(MHD_USE_THREAD_PER_CONNECTION, 8080, NULL, NULL, &handle_request, NULL, MHD_OPTION_END);
    if (daemon == NULL) {
        fprintf(stderr, "Failed to start HTTP server.\n");
        return 1;
    }

    // 使用 select 等待事件
    fd_set read_fds;
    int max_fd = 0;
    
    // 获取标准输入的文件描述符（用于接收用户输入）
    int stdin_fd = STDIN_FILENO;
    max_fd = stdin_fd;

    while (1) {
        FD_ZERO(&read_fds);
        FD_SET(stdin_fd, &read_fds);  // 监听用户输入

        // 使用 select 来等待事件
        int ret = select(max_fd + 1, &read_fds, NULL, NULL, NULL);
        if (ret == -1) {
            perror("select failed");
            break;
        }

        if (FD_ISSET(stdin_fd, &read_fds)) {
            // 处理用户输入
            char input[100];
            if (fgets(input, sizeof(input), stdin) != NULL) {
                printf("User input: %s\n", input);
                // 假设我们输入 "exit" 来退出
                if (strncmp(input, "exit", 4) == 0) {
                    break;
                }
            }
        }

        // 处理 HTTP 请求
        MHD_run(daemon);
    }

    MHD_stop_daemon(daemon);
    return 0;
}

// int main() {
//     // 启动 HTTP 服务器
//     return start_http_server();
// }
