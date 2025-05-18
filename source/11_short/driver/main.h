#ifndef _MAIN_H
#define _MAIN_H

#define MODULE_NAME  "short"
#define SHORT_NDEVS        1
#define SHORT_NR_PORTS     1
#define SHORT_IO_PORT_BASE 0x200
#define SHORT_MMIO_BASE    0xFF000000
#define SHORT_MMIO_LENGTH  0x10


#undef PDEBUG
#ifdef SHORT_DEBUG
#  ifdef __KERNEL__
//   Kernel space debug command
#    define PDEBUG(fmt, args...) printk( KERN_DEBUG "short: " fmt, ## args )
#  else
//   User space debug command
#    define PDEBUG(fmt, args...) fprintf( stderr, "short: " fmt, ## args )
#  endif
#else
// Do nothing
#  define PDEBUG(fnt, args...)
#endif

extern unsigned long short_base;
extern void __iomem *short_iomem;

enum short_types {
    IO_PORT_TYPE,
    REMAP_TYPE,
    MMIO_TYPE,
    NUM_TYPES
};

#endif