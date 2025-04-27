#ifndef _TEST_H
#define _TEST_H

#define NUM_THREADS 8
#define WRITE_BYTES_PER_THREAD 1 << 16

int run_test(char *dev_path);

struct thread_arg {
    int id;
    int fd;
};

#endif