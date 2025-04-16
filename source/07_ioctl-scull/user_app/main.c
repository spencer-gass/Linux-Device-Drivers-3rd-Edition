#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#include "scull-ioctl.h"

static
void print_help(char *progname)
{
    printf("Usage: %s <device_path> <command> [string]\n\n", progname);

    printf("commands:\n");
    printf("  -r, --reset             Reset the device state.\n");
    printf("  -s, --status            Query and print device configuration and status.\n");
    printf("  -a, --append <string>   Append a message string to the device.\n");
    printf("  -h, --help              Show this help message and exit.\n\n");

    printf("Examples:\n");
    printf("  %s /dev/scull -r\n", progname);
    printf("      Resets the device using the IOCTL_RESET command.\n\n");

    printf("  %s /dev/scull -s\n", progname);
    printf("      Prints the current device status.\n\n");

    printf("  %s /dev/scull -a \"hello\"\n", progname);
    printf("      Appends the string \"hello\" to the device.\n\n");

    printf("  %s /dev/scull -h\n", progname);
    printf("      Prints this help text.\n");
}

int main(int argc, char *argv[])
{
    int err = 0;
    int fd = 0;
    char *dev_path = NULL;
    struct ioctl_arg arg = {
        .len = 0,
        .msg = NULL
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
        printf("First argument must be a path to a device in /dev.");
        return -1;
    }

    // Parse command
    if (argc < 3) {
        print_help(argv[0]);
        return -1;
    }

    if (strcmp(argv[2], "-r") == 0 || strcmp(argv[2], "--reset") == 0) {
        fd = open(dev_path, O_RDONLY);
        if (fd < 0) {
            printf("Could not open %s", dev_path);
            return -1;
        }
        err = ioctl(fd, IOCTL_RESET);
        close(fd);
    }
    else if (strcmp(argv[2], "-s") == 0 || strcmp(argv[2], "--status") == 0) {
        fd = open(dev_path, O_RDONLY);
        if (fd < 0) {
            printf("Could not open %s", dev_path);
            return -1;
        }
        err = ioctl(fd, IOCTL_STATUS, &arg);
        printf("%s", arg.msg);
        close(fd);
    }
    else if (strcmp(argv[2], "-a") == 0 || strcmp(argv[2], "--append") == 0) {
        if (argc < 4) {
            printf("Append command requires an additional string argument\n");
            return -1;
        }

        fd = open(dev_path, O_WRONLY);
        if (fd < 0) {
            printf("Could not open %s", dev_path);
            return -1;
        }
        arg.len = strlen(argv[3]);
        arg.msg = argv[3];
        err = ioctl(fd, IOCTL_APPEND, &arg);
        close(fd);
    }
    else {
        printf("Unknown command: %s\n", argv[2]);
        print_help(argv[0]);
        return -1;
    }

    return 0;
}