#!/bin/sh
module="proc-fs-scull"
device="proc-fs-scull"
mode="666"

# get major device number from /proc/devices
major=$(awk -v module="$module" '$2 == module { print $1 }' /proc/devices)

# remove device nodes
rm -f /dev/${device}0
rm -f /dev/${device}1
rm -f /dev/${device}2
rm -f /dev/${device}3

# invoke insmod with all arguments we got and use pathname,
# as new modutils don't look in . by default.
/sbin/rmmod ./$module.ko || exit 1
