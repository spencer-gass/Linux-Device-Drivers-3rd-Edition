#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <fcntl.h>

#include "test.h"

int write_complete = 0;

static void* src_func(void *arg){
    int id = ((struct thread_arg *) arg)->id;
    int fd = ((struct thread_arg *) arg)->fd;
    int retval = 0;
    char wchar;

    for (int i = 0; i < WRITE_BYTES_PER_THREAD; i ++){
        char wchar = 'a' + rand() % 26;
        // printf("src %d, file %d, wchar %c\n", id, fd, wchar);

        retval = write(fd, (void*) &wchar, 1);
        if (retval < 0){
            printf("Failed to write on thread %d\n", id);
        }
    }
}

static void* sink_func(void *arg){
    int id = ((struct thread_arg *) arg)->id;
    int fd = ((struct thread_arg *) arg)->fd;
    int retval = 1;
    char rchar;

    while (!write_complete || retval){
        retval = read(fd, (void*) &rchar, 1);
        if (retval < 0){
            printf("Failed to read on thread %d\n", id);
        } else if (retval >= 0) {
            // printf("sink %d, file %d, rlen %d, rchar %c\n", id, fd, retval, rchar);
        }
    }
}

int run_test(char *dev_path){
    int i;
    ssize_t retval;
    char *wstr = "asdf\n";
    char rstr[256] = {0};
    pthread_t src_threads[NUM_THREADS];
    pthread_t sink_threads[NUM_THREADS];
    struct thread_arg wthread_args[NUM_THREADS];
    struct thread_arg rthread_args[NUM_THREADS];
    int wfd[NUM_THREADS];
    int rfd[NUM_THREADS];

    // Create file descriptors
    for (i = 0; i < NUM_THREADS; i++){
        wfd[i] = open(dev_path, O_WRONLY);
        rfd[i] = open(dev_path, O_RDONLY);
        if (wfd[i] < 0 || rfd[i] < 0) {
            printf("Could not open %s for thread %d\n", dev_path, i);
            printf("wfd: %d, rfd %d\n", wfd[i], rfd[i]);
            return -1;
        }
    }

    write_complete = 0;

    // Launch read and write threads
    for (i = 0; i < NUM_THREADS; i++){
        wthread_args[i].id = i;
        wthread_args[i].fd = wfd[i];
        rthread_args[i].id = i;
        rthread_args[i].fd = rfd[i];
        pthread_create(&src_threads[i],  NULL, src_func,  &wthread_args[i]);
        pthread_create(&sink_threads[i], NULL, sink_func, &rthread_args[i]);
    }

    // Wait for the threads to complete
    for (i = 0; i < NUM_THREADS; i++){
        pthread_join(src_threads[i], NULL);
    }

    write_complete = 1;

    for (i = 0; i < NUM_THREADS; i++){
        pthread_join(sink_threads[i], NULL);
    }

    return retval;
}
