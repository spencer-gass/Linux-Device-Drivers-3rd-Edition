#ifndef _MAIN_H
#define _MAIN_H

#define SCULL_NDEVS 4
#define MODULE_NAME "basic-scull"

struct scull_qset {
    void **data;
    struct scull_qset *next;
};

// root struct that maintains scull device state
struct scull_dev {
    struct scull_qset *data;    // pointer to the first quantum set
    int quantum;                // the current quantum size
    int qset;                   // the current array size
    unsigned long size;         // total amount of data stored
    unsigned int access_key;    // used by sculluid and scullpriv
    struct semaphore sem;       // mutex semaphore
    struct cdev cdev;           // char device struct
};

#endif