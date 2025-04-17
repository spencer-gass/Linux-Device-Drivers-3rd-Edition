#ifndef _SCULL_IOCTL_H
#define _SCULL_IOCTL_H

#define IOCTL_IOC_MAGIC		'd'

#define IOCTL_RESET	    _IO(IOCTL_IOC_MAGIC, 0)
#define IOCTL_STATUS    _IOR(IOCTL_IOC_MAGIC, 1, struct ioctl_arg*)
#define IOCTL_APPEND    _IOW(IOCTL_IOC_MAGIC, 2, struct ioctl_arg*)

#define IOCTL_MAXNR		2
#define MAX_IOCTL_MSG_SIZE 1024

struct ioctl_arg {
    int len;
    char msg[MAX_IOCTL_MSG_SIZE];
};

#endif