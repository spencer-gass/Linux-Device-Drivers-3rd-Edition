#ifndef _MAIN_H
#define _MAIN_H

#define SCULL_NDEVS 4
#define MODULE_NAME "fifo-scull"

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

// root struct that maintains scull device state
struct scull_dev {
    char *data;                 // pointer to the first byte of the data memory
    char *wptr;                 // write pointer
    char *rptr;                 // read pointer
    int mem_size;               // bytes of memory allocated to the data member
    struct semaphore wsem;      // write mutex semaphore
    struct semaphore rsem;      // read mutex semaphore
    struct cdev cdev;           // char device struct
};

#endif