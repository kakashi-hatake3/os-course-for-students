#include <fcntl.h>
#include <pthread.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int channel[2] = {0};

typedef struct {
    int flag;
    char symbol;
} thread_data;

void choose_channel(int whitch) {
    switch (whitch) {
    case 1: {
        pipe(channel);
    } break;
    case 2: {
        pipe2(channel, O_NONBLOCK);
    } break;
    case 3: {
        pipe(channel);
        fcntl(channel[0], F_SETFL, O_NONBLOCK);
        fcntl(channel[1], F_SETFL, O_NONBLOCK);
    } break;
    }
}

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

static void *proc1(void *args) {
    printf("start proc 1\n"); 
    thread_data *thread_args = (thread_data *)args;
    while (thread_args->flag == 0) {
        char *data = get_data();
        ssize_t rv = write(channel[1], data, strlen(data));
        if (rv == 0) {
            printf("EOF\n");
        } else if (rv == -1) {
            perror("err\n");
        } else if (rv > 0) {
            printf("message written\n");
        }
        sleep(1);
    }

    printf("finish proc 1\n");
    return 0;
}

static void *proc2(void *args) {
    printf("start proc 2\n"); 
    thread_data *thread_args = (thread_data *)args;
    char msg[50] = {0};
    while (thread_args->flag == 0) {
        ssize_t rv = read(channel[0], msg, sizeof(msg));
        if (rv == 0) {
            printf("EOF\n");
        } else if (rv == -1) {
            perror("err\n");
        } else if (rv > 0) {
            printf("message received:\n");
            printf(msg);
        }
        sleep(1);
    }
    printf("finish proc 2\n");
    return 0;
}

int main(int argc, char *argv[]) {
    printf("program start\n");
    if (argc != 2) {
        printf("wrong number of arguments\n");
        printf("whitch channel: 1, 2 or 3\n");
        return 1;
    } else {
        int whitch = atoi(argv[1]);
        choose_channel(whitch);
    }
    pthread_t first_thread;
    pthread_t second_thread;

    thread_data first_struct;
    thread_data second_struct;
    first_struct.flag = 0;
    second_struct.flag = 0;

    pthread_create(&first_thread, NULL, proc1, &first_struct);
    pthread_create(&second_thread, NULL, proc2, &second_struct);

    printf("waiting for any key\n");
    getchar();

    first_struct.flag = 1;
    second_struct.flag = 1;

    pthread_join(first_thread, NULL);
    pthread_join(second_thread, NULL);
    printf("both treads have finished\n");

    close(channel[0]);
    close(channel[1]);
    printf("end\n");
    return 0;
}
