#include <arpa/inet.h>
#include <fcntl.h>
#include <iostream>
#include <netinet/in.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>

int soc = 0;
const char *ip_server = "127.0.0.1";
const int port_server = 13370;
pthread_t id_req, id_rec, id_wait;
int flag_req = 0;
int flag_rec = 0;
int flag_w = 0;

void *request(void *args) {
    int number = 0;
    while (flag_req == 0) {
        char req[100] = {0};
        sprintf(req, "%d", number++);
        int ret = send(soc, req, sizeof(req), 0);
        if (ret == -1) {
            perror("send");
        } else {
        }
        printf("sending request %s\n", req);
        sleep(1);
    }
    pthread_exit((void *)7);
}

void *receive(void *args) {
    while (flag_rec == 0) {
        char res[100] = {0};
        int ret = recv(soc, (void *)&res, sizeof(res), 0);
        if (ret == -1) {
            perror("recv");
            sleep(1);
        } else if (ret == 0) {
            puts("server disconnected");
            pthread_exit((void *)8);
        } else {
            printf("received responce: %s\n", res);
        }
    }
    pthread_exit((void *)8);
}

void *wait(void *args) {
    while (flag_w == 0) {
        struct sockaddr_in serverAddr;
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = ntohs(port_server);
        serverAddr.sin_addr.s_addr = inet_addr(ip_server);
        int result =
            connect(soc, (struct sockaddr *)&serverAddr, sizeof(serverAddr));
        if (result == -1) {
            perror("connect");
            sleep(1);
        } else {
            struct sockaddr_in soc_addr;
            socklen_t len = sizeof(soc_addr);
            getsockname(soc, (struct sockaddr *)&soc_addr, &len);
            char ip[INET_ADDRSTRLEN];
            uint16_t port;
            inet_ntop(AF_INET, &soc_addr, ip, sizeof(ip));
            port = htons(soc_addr.sin_port);
            printf("connected to server, socket address %s, port: %d\n", ip,
                   port);
            pthread_create(&id_req, NULL, request, NULL);
            pthread_create(&id_rec, NULL, receive, NULL);
            pthread_exit((void *)8);
        }
    }
    pthread_exit((void *)1337);
}

int main() {
    puts("client started");
    soc = socket(AF_INET, SOCK_STREAM, 0);
    int optval = 1;
    setsockopt(soc, SOL_SOCKET, SO_REUSEADDR | SO_KEEPALIVE, &optval,
               sizeof(optval));
    fcntl(soc, F_SETFL, O_NONBLOCK);

    pthread_create(&id_wait, NULL, wait, NULL);
    puts("client is waiting for keyboard input");
    getchar();
    puts("keyboard input received");

    flag_req = 1;
    flag_rec = 1;
    flag_w = 1;

    pthread_join(id_req, NULL);
    pthread_join(id_rec, NULL);
    pthread_join(id_wait, NULL);

    shutdown(soc, SHUT_RDWR);
    close(soc);
    puts("client finished");
    return 0;
}
