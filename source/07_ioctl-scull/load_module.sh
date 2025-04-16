#!/bin/sh
module="ioctl-scull"
device="ioctl-scull"
mode="666"

# invoke insmod with all arguments we got and use pathname,
# as new modutils don't look in . by default.
/sbin/insmod ./$module.ko $* || exit 1

# remove stale nodes
rm -f /dev/${device}[0-3]

# get major device number from /proc/devices
major=$(awk -v module="$module" '$2 == module { print $1 }' /proc/devices)

# make device nodes
mknod /dev/${device}0 c $major 0
mknod /dev/${device}1 c $major 1
mknod /dev/${device}2 c $major 2
mknod /dev/${device}3 c $major 3

# give appropriate group/permissions and change the group.
# not all distributions have staff, some have wheel instead.
# group="staff"
# grep -q '^staff:' /etc/group || group="wheel"

# chgrp $group /dev/${device}[0-3]
chmod $mode  /dev/${device}[0-3]

