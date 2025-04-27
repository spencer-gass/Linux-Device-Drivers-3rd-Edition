# Scull Module with Selectible Mutex

- Inherits from fifo-scull
- Add ioctl commands to select between semaphore and spinlock mutexes
- Add test app that compares performance between the two mutex types

### Results

Parameters:
CPU cores:                  4
Read Threads:               8
Write Threads:              8
Bytes written per thread:   2^16

```
/mnt/source/10_mutex-scull/bin # ./load_module.sh
[   12.334867] mutex_scull: loading out-of-tree module taints kernel.
[   12.335296] mutex-scull loaded
[   12.335458] Loaded mutex-scull0
[   12.335611] Loaded mutex-scull1
[   12.335789] Loaded mutex-scull2
[   12.335937] Loaded mutex-scull3
/mnt/source/10_mutex-scull/bin # ./scull-app /dev/mutex-scull0 -s
Driver status: Device numbers 247:0
FIFO memory size:       4096
Write Pointer:          0
Read Pointer:           0
Occupancy:              0
Free Bytes:             4096
Write Mutex:            Semaphore
Write Semaphore State:  1
Read Mutex:             Semaphore
Read Semaphore State:   1

/mnt/source/10_mutex-scull/bin # time ./scull-app /dev/mutex-scull0 -t
real	0m 3.45s
user	0m 0.20s
sys	0m 8.39s
/mnt/source/10_mutex-scull/bin # time ./scull-app /dev/mutex-scull0 -t
real	0m 5.46s
user	0m 0.48s
sys	0m 13.17s
/mnt/source/10_mutex-scull/bin # time ./scull-app /dev/mutex-scull0 -t
real	0m 5.52s
user	0m 0.50s
sys	0m 13.31s
/mnt/source/10_mutex-scull/bin # ./scull-app /dev/mutex-scull0 -r
/mnt/source/10_mutex-scull/bin # ./scull-app /dev/mutex-scull0 --set-mutex-spinlock
/mnt/source/10_mutex-scull/bin # time ./scull-app /dev/mutex-scull0 -t
real	0m 3.73s
user	0m 0.28s
sys	0m 8.88s
/mnt/source/10_mutex-scull/bin # time ./scull-app /dev/mutex-scull0 -t
real	0m 5.58s
user	0m 0.51s
sys	0m 13.45s
/mnt/source/10_mutex-scull/bin # time ./scull-app /dev/mutex-scull0 -t
real	0m 5.54s
user	0m 0.54s
sys	0m 13.28s
/mnt/source/10_mutex-scull/bin # time ./scull-app /dev/mutex-scull0 -t
real	0m 5.49s
user	0m 0.53s
sys	0m 13.11s
/mnt/source/10_mutex-scull/bin # ./scull-app /dev/mutex-scull0 -r
/mnt/source/10_mutex-scull/bin # ./scull-app /dev/mutex-scull0 --set-mutex-semaphore
/mnt/source/10_mutex-scull/bin # ./scull-app /dev/mutex-scull0 -s
Driver status: Device numbers 247:0
FIFO memory size:       4096
Write Pointer:          0
Read Pointer:           0
Occupancy:              0
Free Bytes:             4096
Write Mutex:            Semaphore
Write Semaphore State:  1
Read Mutex:             Semaphore
Read Semaphore State:   1

/mnt/source/10_mutex-scull/bin # time ./scull-app /dev/mutex-scull0 -t
real	0m 3.95s
user	0m 0.33s
sys	0m 9.33s
/mnt/source/10_mutex-scull/bin # time ./scull-app /dev/mutex-scull0 -t
real	0m 5.50s
user	0m 0.56s
sys	0m 13.01s
/mnt/source/10_mutex-scull/bin # time ./scull-app /dev/mutex-scull0 -t
real	0m 5.45s
user	0m 0.53s
sys	0m 12.95s
/mnt/source/10_mutex-scull/bin

```

### Conclusion

 - Similar results between semaphore and spinlock
 - Test time increased when the test was repeated imediatly.
   Not sure what was causing this and didn't take the time to investigate.