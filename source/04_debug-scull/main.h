#ifndef _MAIN_H
#define _MAIN_H

#define SCULL_NDEVS 4
#define MODULE_NAME "debug-scull"

#undef PDEBUG
#ifdef SCULL_DEBUG
#  ifdef __KERNEL__
//   Kernel space debug command
#    define PDEBUG(fmt, args...) printk( KERN_DEBUG "scull: " fmt, ## args )
#  else
//   User space debug command
#    define PDEBUG(fmt, args...) fprintf( stderr, "scull: " fmt, ## args )
#  endif
#else
// Do nothing
#  define PDEBUG(fnt, args...)
#endif

struct scull_list_node {
    void **data;
    struct scull_list_node *next;
};

// root struct that maintains scull device state
struct scull_dev {
    struct scull_list_node *data;   // pointer to the first list node
    int block_size;                 // bytes per block
    int block_list_size;            // blocks per array
    unsigned long size;             // total bytes in the data structure
    unsigned int access_key;        // used by sculluid and scullpriv
    struct semaphore sem;           // mutex semaphore
    struct cdev cdev;               // char device struct
};

#endif