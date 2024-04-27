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

void *proc(void *args) {
    while (flag == 0) {
        char *data = get_data();
        printf("Result is %c\n", data);

        Message msg;
        msg.msg_type = 1;
        int len = sprintf(msg.buff, "%c", data);

        int ret = msgsnd(msgq_fd, &msg, len, IPC_NOWAIT);
        if (ret == 0) {
            // success
        } else {
            perror("send");
        }

        sleep(1);
    }
    pthread_exit((void *)8);
}

int main() {
    puts("Writer program started working...");

    pthread_t id;
    key_t key = ftok("file.txt", '8');

    msgq_fd = msgget(key, 0);
    if (msgq_fd < 0) {
        msgq_fd = msgget(key, IPC_CREAT | S_IRUSR | S_IWUSR);
    }

    pthread_create(&id, NULL, proc, NULL);

    puts("Writer program is waiting for keyboard input...");
    getchar();
    puts("Keyboard input received!");

    flag = 1;

    int ret = 0;
    pthread_join(id, (void **)&ret);

    msgctl(msgq_fd, IPC_RMID, NULL);

    puts("Writer program finished working!");
    return 0;
}
