#include <iostream>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>

struct thread_data
{
    int flag;
    int exit_code;
};

static void * proc1(void* arg){
    printf("start proc 1\n"); 
    thread_data* data = (thread_data*) arg;
    while(data->flag != 0){
        putchar('1');
        fflush(stdout);
        sleep(1);
    }


    printf("finish proc 1\n");

    pthread_exit(&data->exit_code);

}

static void * proc2(void* arg){
    printf("start proc 2\n"); 


    sigset_t set;
    siginfo_t info;
    union sigval value;

    sigemptyset(&set);
    sigaddset(&set, SIGUSR1);
    pthread_sigmask(SIG_BLOCK, &set, NULL);



    thread_data* data = (thread_data*) arg;
    while(data->flag != 0){
        putchar('2');
        fflush(stdout);
        sleep(1);
    }

    sigwaitinfo(&set, &info);

    printf("signal was taken in 2nd thread: id: %d, value: %d\n", info.si_signo, info.si_value.sival_int);


    printf("finish proc 2\n");
    

    pthread_exit(&data->exit_code);

}

int main(){
    printf("program start\n");
    thread_data first_struct;
    thread_data second_struct;

    union sigval value;

    value.sival_int = 666;

    first_struct.flag = 1;
    second_struct.flag = 1;
    first_struct.exit_code = 201;
    second_struct.exit_code = 202;

    pthread_t first_thread;
    pthread_t second_thread;

    pthread_create(&first_thread, NULL, proc1, &first_struct.flag);
    pthread_create(&second_thread, NULL, proc2, &second_struct.flag);



    printf("waiting for any key\n");
    getchar();



    first_struct.flag = 0;
    second_struct.flag = 0;

    pthread_sigqueue(second_thread, SIGUSR1, value);

    int* result1;
    int* result2;



    pthread_join(first_thread, (void **)&result1);
    pthread_join(second_thread, (void **)&result2);


    printf("both treads have finished\n");
    printf("proc1 exit code: %d\n", *result1);
    printf("proc2 exit code: %d\n", *result2);
    printf("end\n");

    return 0;

}
