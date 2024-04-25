#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>
#include <pwd.h>

int flag = 0;
sem_t *write_sem;
const char *write_sem_name = "/write_sem";
sem_t *read_sem;
const char *read_sem_name = "/read_sem";
int shmem_fd;
void *local_addr = malloc(1000);

void *proc1(void *args) {
    puts("proc1 started");
    while (flag == 0) {
        char data[MEM_SIZE] = {0};
        printf("Result is: %d\n", data);
        memcpy(&data, local_addr, sizeof(data));
        fflush(stdout);

        sem_post(write_sem);
        sem_wait(read_sem);
        sleep(1);
    }

    puts("proc1 finished");
    pthread_exit((void *)7);
}

void sig_handler(int signo) {
    printf("\nget SIGNINT; %d\n", signo);
    sem_close(write_sem);
    sem_unlink(write_sem_name);
    sem_close(read_sem);
    sem_unlink(read_sem_name);

    munmap(local_addr, 100);
    close(shmem_fd);
    shm_unlink("/memory");
    exit(0);
}

int main() {
    puts("main program 1 started");
    signal(SIGINT, sig_handler);
    pthread_t id;

    shmem_fd = shm_open("/memory", O_RDWR | O_CREAT, S_IRWXU);
    ftruncate(shmem_fd, sizeof(int));
    local_addr = mmap(local_addr, sizeof(int), PROT_WRITE | PROT_READ,
                      MAP_SHARED, shmem_fd, 0);
    write_sem = sem_open(write_sem_name, O_CREAT, (mode_t)0777, 0);
    read_sem = sem_open(read_sem_name, O_CREAT, (mode_t)0777, 0);

    pthread_create(&id, NULL, proc1, NULL);

    puts("Main program 1 is waiting for keyboard input...");
    getchar();
    puts("Keyboard input received!");

    flag = 1;

    int ret = 0;
    pthread_join(id, (void **)&ret);

    sem_close(write_sem);
    sem_unlink(write_sem_name);
    sem_close(read_sem);
    sem_unlink(read_sem_name);

    munmap(local_addr, 100);
    close(shmem_fd);
    shm_unlink("/memory");

    printf("proc1 returned: %d\n", ret);

    puts("main program 1 finished");
    return 0;
}
