#include <arpa/inet.h>
#include <fcntl.h>
#include <iostream>
#include <netinet/in.h>
#include <pthread.h>
#include <pwd.h>
#include <queue>
#include <semaphore.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>

int soc_listen = 0;
int soc_client = 0;
int soc_resp = 0;
std::queue<std::string> req_queue;
int flag_a = 0;
int flag_p = 0;
int flag_w = 0;
pthread_mutex_t mutex;
pthread_t id_accept, id_process;
pthread_t id_wait;
const int port_listen = 13370;

char *get_data() {
    struct passwd pwd;
    struct passwd *result;
    char* name = "Username";
    char buffer[1024];
    int bufflen = sizeof(buffer);
    if (getpwnam_r(name, &pwd, buffer, bufflen, &result) != 0 || result == NULL){
        perror("getpwnam_r");
        exit(EXIT_FAILURE);
    }
    return pwd.pw_dir;
}

void *accept(void *args) {
    while (flag_a == 0) {
        char data[100];
        int ret = recv(soc_client, (void *)&data, sizeof(data), 0);
        if (ret == -1) {
            // error
            perror("recv");
            sleep(1);
        } else if (ret == 0) {
            puts("client disconected");
            pthread_exit((void *)69);
        } else {
            pthread_mutex_lock(&mutex);
            req_queue.push(std::string(data));
            pthread_mutex_unlock(&mutex);
            printf("request: %s\n", data);
        }
    }
    pthread_exit((void *)69);
}

void *process(void *args) {
    while (flag_p == 0) {
        pthread_mutex_lock(&mutex);
        if (!req_queue.empty()) {
            std::string req = req_queue.front();
            req_queue.pop();
            pthread_mutex_unlock(&mutex);
            char *data = get_data();
            std::string res = req + " " + std::string(data);
            int ret = send(soc_client, res.c_str(), strlen(res.c_str()), 0);
            if (ret == -1) {
                perror("send");
            } else {
                printf("sending response: %s\n", res.c_str());
            }
        } else {
            pthread_mutex_unlock(&mutex);
        }
    }
    pthread_exit((void *)28);
}

void *wait(void *args) {
    while (flag_w == 0) {
        struct sockaddr_in client;
        socklen_t len = sizeof(client);
        int result = accept(soc_listen, (struct sockaddr *)&client, &len);
        if (result == -1) {
            perror("accept");
            sleep(1);
        } else {
            soc_client = result;
            char ip[INET_ADDRSTRLEN];
            uint16_t port;
            inet_ntop(AF_INET, &client, ip, sizeof(ip));
            port = htons(client.sin_port);
            printf("client connected, client address %s, client port %d\n", ip,
                   port);
            pthread_create(&id_accept, NULL, accept, NULL);
            pthread_create(&id_process, NULL, process, NULL);
            pthread_exit((void *)8);
        }
    }
    pthread_exit((void *)7);
}

void sigpipe_handler(int signo) {
    puts("client disconnected");
    shutdown(soc_client, SHUT_RDWR);
    close(soc_client);
    close(soc_listen);
    exit(0);
}

int main() {
    puts("server started");
    signal(SIGPIPE, sigpipe_handler);

    soc_listen = socket(AF_INET, SOCK_STREAM, 0);
    int option = 1;
    setsockopt(soc_listen, SOL_SOCKET, SO_REUSEADDR | SO_KEEPALIVE, &option,
               sizeof(option));
    fcntl(soc_listen, F_SETFL, O_NONBLOCK);

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port_listen);

    bind(soc_listen, (struct sockaddr *)&addr, sizeof(addr));
    listen(soc_listen, 1);

    pthread_create(&id_wait, NULL, wait, NULL);
    puts("server is waiting for keyboard input");
    getchar();
    puts("keyboard input received");

    flag_a = 1;
    flag_p = 1;
    flag_w = 1;

    pthread_join(id_accept, NULL);
    pthread_join(id_process, NULL);
    pthread_join(id_wait, NULL);

    shutdown(soc_client, SHUT_RDWR);
    close(soc_client);
    close(soc_listen);
    puts("server finished");
    return 0;
}
