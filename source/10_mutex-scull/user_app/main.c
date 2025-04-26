#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#include "scull-ioctl.h"
#include "test.h"

static
void print_help(char *progname)
{
    printf("Usage: %s <device_path> <command>\n\n", progname);

    printf("commands:\n");
    printf("  -r, --reset             Resets device state.\n");
    printf("  -s, --status            Prints device status.\n");
    printf("  --set-mutex-semaphore   Sets the read and write mutexs to use semaphores.\n");
    printf("  --set-mutex-spinlock    Sets the read and write mutexs to use spinlocks.\n");
    printf("  -t, --test              Runs a perfomance test comparing the two mutex types.\n");
    printf("  -h, --help              Show this help message and exit.\n\n");

    printf("Examples:\n");
    printf("  %s /dev/scull -r\n", progname);
    printf("      Resets the device using the IOCTL_RESET command.\n\n");

    printf("  %s /dev/scull -s\n", progname);
    printf("      Prints the current device status.\n\n");

    printf("  %s /dev/scull --set-mutex-semaphore\n\n", progname);
    printf("      Sets the read and write mutexs to use semaphores.\n\n");

    printf("  %s /dev/scull --set-mutex-spinlock\n\n", progname);
    printf("      Sets the read and write mutexs to use spinlocks.\n\n");

    printf("  %s /dev/scull -t\n", progname);
    printf("      Runs a perfomance test comparing the two mutex types.\n\n");

    printf("  %s /dev/scull -h\n", progname);
    printf("      Prints this help text.\n");
}

int main(int argc, char *argv[])
{
    int err = 0;
    int fd = 0;
    char *dev_path = NULL;
    size_t input_len = 0;
    struct ioctl_status status = {
        .len = 0,
        .msg = ""
    };

    if (argc < 2) {
        print_help(argv[0]);
        return -1;
    }

    // Check args for help opiton
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            print_help(argv[0]);
            return 0;
        }
    }

    // Normal operation
    // Parse device path
    // Verify first arg is a string that represents a path to a device in /dev
    if (strncmp(argv[1], "/dev", 4) == 0){
        dev_path = argv[1];
    } else {
        printf("First argument must be a path to a device in /dev.\n");
        return -1;
    }

    // Parse command
    if (argc < 3) {
        printf("Too few arguments.\n");
        print_help(argv[0]);
        return -1;
    }

    if (strcmp(argv[2], "-r") == 0 || strcmp(argv[2], "--reset") == 0) {
        fd = open(dev_path, O_RDONLY);
        if (fd < 0) {
            printf("Could not open %s\n", dev_path);
            return -1;
        }
        err = ioctl(fd, IOCTL_RESET);
        close(fd);
    }
    else if (strcmp(argv[2], "-s") == 0 || strcmp(argv[2], "--status") == 0) {
        fd = open(dev_path, O_RDONLY);
        if (fd < 0) {
            printf("Could not open %s\n", dev_path);
            return -1;
        }
        err = ioctl(fd, IOCTL_STATUS, &status);
        printf("Driver status: %s\n", status.msg);
        close(fd);
    }
    else if (strcmp(argv[2], "--set-mutex-semaphore") == 0) {
        struct ioctl_mutex_config mutex_config = {
            .wr_mutex_type = SEMAPHORE,
            .rd_mutex_type = SEMAPHORE
        };
        fd = open(dev_path, O_WRONLY);
        if (fd < 0) {
            printf("Could not open %s\n", dev_path);
            return -1;
        }
        err = ioctl(fd, IOCTL_SET_MUTEX_T, &mutex_config);
        close(fd);
    }
    else if (strcmp(argv[2], "--set-mutex-spinlock") == 0) {
        struct ioctl_mutex_config mutex_config = {
            .wr_mutex_type = SPINLOCK,
            .rd_mutex_type = SPINLOCK
        };
        fd = open(dev_path, O_WRONLY);
        if (fd < 0) {
            printf("Could not open %s\n", dev_path);
            return -1;
        }
        err = ioctl(fd, IOCTL_SET_MUTEX_T, &mutex_config);
        close(fd);
    }
    else if (strcmp(argv[2], "-t") == 0 || strcmp(argv[2], "--test") == 0) {
        run_test(dev_path);
    }
    else {
        printf("Unknown command: %s\n", argv[2]);
        print_help(argv[0]);
        return -1;
    }

    return 0;
}