#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/epoll.h>

#include "worker.h"
#include "utils.h"
#include "stats.h"

#define PORT 8080

int main() {
    listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd < 0) { perror("socket"); return 1; }

    int opt = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    set_nonblocking(listen_fd);

    struct sockaddr_in serv_addr = {0};
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(PORT);
    if (bind(listen_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("bind"); return 1;
    }
    if (listen(listen_fd, SOMAXCONN) < 0) {
        perror("listen"); return 1;
    }

    int epoll_fd = epoll_create1(0);
    if (epoll_fd < 0) { perror("epoll_create1"); return 1; }
    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = listen_fd;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, listen_fd, &ev);

    pthread_t rps_thread;
    pthread_create(&rps_thread, NULL, rps_counter, NULL);
    pthread_detach(rps_thread);

    int nthreads = sysconf(_SC_NPROCESSORS_ONLN);
    if (nthreads < 1) nthreads = 2;
    pthread_t *threads = malloc(nthreads * sizeof(pthread_t));
    for (int i = 0; i < nthreads; i++) {
        pthread_create(&threads[i], NULL, worker, &epoll_fd);
        pthread_detach(threads[i]);
    }

    printf("Server listening on port %d ...\n", PORT);
    while (1) pause();

    close(listen_fd);
    return 0;
}
