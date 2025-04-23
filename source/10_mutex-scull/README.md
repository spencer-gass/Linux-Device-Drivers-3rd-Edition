# Scull Module with Selectible Mutex

- Inherits from fifo-scull
- Add ioctl commands to select between semaphore and spinlock mutexes
- Add test app that compares performance between the two mutex types

### Usage

```
./load_module.sh
./run_test.sh
./unload_module.sh
```