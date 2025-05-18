#!/bin/sh
module="short"
io_port_type=0
remap_type=1
mmio_type=2

/sbin/rmmod ./$module.ko TYPE=$io_port_type || exit 1
/sbin/rmmod ./$module.ko TYPE=$remap_type   || exit 1
/sbin/rmmod ./$module.ko TYPE=$mmio_type    || exit 1
