#ifndef _SCULL_IOCTL_H
#define _SCULL_IOCTL_H

#define IOCTL_IOC_MAGIC		'd'

#define IOCTL_RESET	        _IO(IOCTL_IOC_MAGIC, 0)
#define IOCTL_STATUS        _IOR(IOCTL_IOC_MAGIC, 1, struct ioctl_status*)
#define IOCTL_SET_MUTEX_T   _IOW(IOCTL_IOC_MAGIC, 2, struct ioctl_mutex_config*)

#define IOCTL_MAXNR		    2
#define MAX_IOCTL_MSG_SIZE  512

#define IS_KERNEL_BUF   0
#define IS_USER_BUF	    1

#define SEMAPHORE 0
#define SPINLOCK  1

struct ioctl_status {
    int len;
    char msg[MAX_IOCTL_MSG_SIZE];
};

struct ioctl_mutex_config {
    char wr_mutex_type;
    char rd_mutex_type;
};

#endif