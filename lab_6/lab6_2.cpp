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

void *proc2(void *args) {
    puts("proc2 started");
    while (flag == 0) {
        sem_wait(write_sem);
        char *data = get_data();
        memcpy(local_addr, data, sizeof(int));
        printf("Result is: %d\n", data);
        fflush(stdout);
        sem_post(read_sem);
        sleep(1);
    }

    puts("proc2 finished");
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
    puts("main program 2 started");
    signal(SIGINT, sig_handler);
    pthread_t id;

    shmem_fd = shm_open("/memory", O_RDWR | O_CREAT, S_IRWXU);
    ftruncate(shmem_fd, 100);
    local_addr =
        mmap(local_addr, 100, PROT_WRITE | PROT_READ, MAP_SHARED, shmem_fd, 0);
    write_sem = sem_open(write_sem_name, O_CREAT, (mode_t)0777, 0);
    read_sem = sem_open(read_sem_name, O_CREAT, (mode_t)0777, 0);

    pthread_create(&id, NULL, proc2, NULL);

    puts("Main program 2 is waiting for keyboard input...");
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

    printf("proc2 returned: %d\n", ret);

    puts("main program 2 finished");
    return 0;
}
