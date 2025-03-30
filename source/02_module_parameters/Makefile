# Check if the KERNELRELEASE variable is set to determine the build context
ifneq ($(KERNELRELEASE),)
    # Use the MODULE variable to specify which module to build
    obj-m := module-parameters.o

# Fallback for direct invocation from the command line
else

# Set the kernel directory and current working directory
KERNELDIR ?= ${LDD_ROOT}/kernel/linux-6.8
PWD := $(shell pwd)

# Default target to build the specified module
default:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

# Clean target to remove build artifacts
# clean:
#	 rm -f *.o *.ko *.mod.* *.mod modules.order Module.symvers

.PHONY: clean
clean:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) clean

endif
