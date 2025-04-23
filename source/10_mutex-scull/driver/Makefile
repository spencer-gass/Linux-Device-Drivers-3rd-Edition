# Uncomment to enable debug printks
DEBUG = y

ifeq ($(DEBUG),y)
	DEBFLAGS = -O -g -DSCULL_DEBUG
else
	DEBFLAGS = -O2
endif

# Global CFLAGS
ccflags-y := $(DEBFLAGS)
# Per file CFLAGS
CFLAGS_main.o := $(DEBFLAGS)
CFLAGS_scull-file-ops.o := $(DEBFLAGS)

# Check if the KERNELRELEASE variable is set to determine the build context
ifneq ($(KERNELRELEASE),)
	mutex-scull-objs := main.o scull-file-ops.o
    obj-m := mutex-scull.o

# Fallback for direct invocation from the command line
else

# Set the kernel directory and current working directory
KERNELDIR ?= ${LDD_ROOT}/kernel/linux-6.8
PWD := $(shell pwd)

# Default target to build the specified module
default:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

# Clean target to remove build artifacts
.PHONY: clean
clean:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) clean

endif
