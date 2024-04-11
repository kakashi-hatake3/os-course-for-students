#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>

int main() {
    puts("Program 1 started");
    fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);

    const char *sem_name = "/svetophor";
    FILE *file;
    sem_t *sem = sem_open(sem_name, O_CREAT, (mode_t)0777, 1);
    file = fopen("./file.txt", "a+");

    fd_set fds;
    timeval timeout;

    while (1) {
        FD_ZERO(&fds);
        FD_SET(0, &fds);

        timeout.tv_sec = 1;
        timeout.tv_usec = 0;

        sem_wait(sem);
        for (int i = 0; i < 10; i++) {
            fprintf(file, "%c", '1');
            putchar('1');
            fflush(file);
            fflush(stdout);
            sleep(1);
        }
        sem_post(sem);

        if (select(1, &fds, NULL, NULL, &timeout) > 0) {
            printf("\nkey pressed\n");
            break;
        } else {
            continue;
        }
    }

    fclose(file);
    sem_close(sem);
    sem_unlink(sem_name);

    puts("Program 1 finished!");
    return 0;
}
