#include <iostream>
#include <pthread.h>
#include <unistd.h>
#include <csignal>

typedef struct {
    int flag;
} thread_data;


pthread_mutex_t mutex;

void sig_handler(int sig) {
    printf("\nget SIGINT; %d\n", sig);
    exit(0);
}


static void* proc1(void* arg) {
    printf("start proc 1\n"); 
    thread_data* data = (thread_data*) arg;
    while (data->flag != 0) {   
        pthread_mutex_lock(&mutex);
        for (int i = 0; i < 10; ++i) {
            putchar('1');
            fflush(stdout);
            sleep(1);
        } 

        pthread_mutex_unlock(&mutex);
        sleep(1);
    }

    printf("finish proc 1\n");

    pthread_exit((void*)0);
}

static void* proc2(void* arg) {
    printf("start proc 2\n"); 

    thread_data* data = (thread_data*) arg;
    while (data->flag != 0) {
        pthread_mutex_lock(&mutex);
        for (int i = 0; i < 10; ++i) {
            putchar('2');
            fflush(stdout);
            sleep(1);
        }
        pthread_mutex_unlock(&mutex);
        sleep(1);
    }

    printf("finish proc 2\n");
    
    pthread_exit((void*)0);
}

int main() {
    printf("program start\n");

    signal(SIGINT, sig_handler);

    pthread_t first_thread;
    pthread_t second_thread;

    thread_data first_struct;
    thread_data second_struct;

    pthread_mutex_t first_mutex;

    first_struct.flag = 1;
    second_struct.flag = 1;
    
    printf("init mutex \n");
    pthread_mutex_init(&mutex, NULL);

    pthread_create(&first_thread, NULL, proc1, &first_struct);
    pthread_create(&second_thread, NULL, proc2, &second_struct);

    printf("waiting for any key\n");
    getchar();

    first_struct.flag = 0;
    second_struct.flag = 0;
    
    pthread_join(first_thread, NULL);
    pthread_join(second_thread, NULL);

    printf("both treads have finished\n");

    printf("destroy mutex \n");
    pthread_mutex_destroy(&mutex);

    printf("end\n");
    return 0;
}