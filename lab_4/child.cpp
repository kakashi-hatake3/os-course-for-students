#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    printf("child started\n");
    pid_t pid = getpid();
    pid_t ppid = getppid();
    printf("pid: %d, parent pid: %d\n", pid, ppid);

    printf("args:\n");
    for (int i = 0; i < argc; i++) {
        puts(argv[i]);
    }
    printf("child finished\n");
    exit(69);
}
