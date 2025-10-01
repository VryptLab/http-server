#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <stdatomic.h>

#include "worker.h"
#include "utils.h"
#include "stats.h"

#define MAX_EVENTS 1024
#define READ_BUFFER 4096

int listen_fd;

void *worker(void *arg) {
    int epoll_fd = *(int*)arg;
    struct epoll_event events[MAX_EVENTS];
    char buffer[READ_BUFFER];

    while (1) {
        int n = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        if (n < 0) {
            if (errno == EINTR) continue;
            perror("epoll_wait");
            break;
        }

        for (int i = 0; i < n; i++) {
            int fd = events[i].data.fd;
            if (fd == listen_fd) {
                while (1) {
                    struct sockaddr_in client_addr;
                    socklen_t addrlen = sizeof(client_addr);
                    int client_fd = accept(listen_fd, (struct sockaddr*)&client_addr, &addrlen);
                    if (client_fd < 0) {
                        if (errno == EAGAIN || errno == EWOULDBLOCK) break;
                        perror("accept");
                        break;
                    }
                    set_nonblocking(client_fd);
                    struct epoll_event ev;
                    ev.events = EPOLLIN;
                    ev.data.fd = client_fd;
                    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &ev);
                    atomic_fetch_add(&total_connections, 1);
                }
            } else {
                int client_fd = fd;
                int len = read(client_fd, buffer, READ_BUFFER - 1);
                if (len <= 0) {
                    close(client_fd);
                    continue;
                }
                buffer[len] = '\0';
                char method[8], path[128];
                if (sscanf(buffer, "%s %s", method, path) < 2) {
                    close(client_fd);
                    continue;
                }
                char response[256];
                char header[128];
                if (strcmp(method, "GET") == 0) {
                    if (strcmp(path, "/") == 0) {
                        const char *body = "OK";
                        int body_len = strlen(body);
                        snprintf(header, sizeof(header),
                            "HTTP/1.1 200 OK\r\n"
                            "Content-Type: text/plain\r\n"
                            "Content-Length: %d\r\n"
                            "Connection: close\r\n\r\n",
                            body_len);
                        write(client_fd, header, strlen(header));
                        write(client_fd, body, body_len);
                    }
                    else if (strcmp(path, "/stats") == 0) {
                        long total = atomic_load(&total_connections);
                        int rps = atomic_load(&last_rps);
                        int body_len = snprintf(response, sizeof(response),
                            "{\"total_connections\": %ld, \"rps\": %d}", total, rps);
                        snprintf(header, sizeof(header),
                            "HTTP/1.1 200 OK\r\n"
                            "Content-Type: application/json\r\n"
                            "Content-Length: %d\r\n"
                            "Connection: close\r\n\r\n",
                            body_len);
                        write(client_fd, header, strlen(header));
                        write(client_fd, response, body_len);
                    }
                }
                atomic_fetch_add(&current_requests, 1);
                epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client_fd, NULL);
                close(client_fd);
            }
        }
    }
    return NULL;
}
