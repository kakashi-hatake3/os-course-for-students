#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/mman.h>
#include <sys/msg.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>

int flag = 0;
int msgq_fd = 0;

typedef struct {
    long msg_type;
    char buff[256];
} Message;

void *proc(void *args) {
    Message msg;
    while (flag == 0) {
        msg.msg_type = 1;
        memset(msg.buff, 0, sizeof(msg.buff));

        int ret =
            msgrcv(msgq_fd, &msg, sizeof(msg.buff), msg.msg_type, IPC_NOWAIT);

        if (ret == -1) {
            perror("receive");
        } else {
            printf("Received message: %s\n", msg.buff);
        }
        sleep(1);
    }
    pthread_exit((void *)8);
}

int main() {
    puts("Reader program started working...");

    pthread_t id;
    key_t key = ftok("file.txt", '8');

    msgq_fd = msgget(key, 0);
    if (msgq_fd < 0) {
        msgq_fd = msgget(key, IPC_CREAT | S_IRUSR | S_IWUSR);
    }

    pthread_create(&id, NULL, proc, NULL);

    puts("Reader program is waiting for keyboard input...");
    getchar();
    puts("Keyboard input received!");

    flag = 1;

    int ret = 0;
    pthread_join(id, (void **)&ret);

    msgctl(msgq_fd, IPC_RMID, NULL);

    puts("Reader program finished working!");
    return 0;
}
