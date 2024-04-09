#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    printf("parent started\n");
    pid_t pid = getpid();
    pid_t ppid = getppid();
    pid_t child = fork();
    if (child == 0) {
        printf("here\n");
        int ret = execlp("child", argv[0], argv[1], argv[2], NULL);
        if (ret == -1) {
            perror("child");
        }
    } else if (child > 0) {
        printf("parent: pid: %d, parent pid: %d, child pid: %d\n", pid, ppid,
               child);
        int code = 0;
        while (waitpid(child, &code, WNOHANG) == 0) {
            usleep(500000);
        }
        printf("parent: child exit code: %d\n", WEXITSTATUS(code));
    } else {
        perror("fork");
    }

    printf("parent finished\n");
    return 0;
}
