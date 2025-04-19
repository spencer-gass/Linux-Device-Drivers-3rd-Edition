# Scull Module with a Circular Buffer in Place of the Linked List

- Inherits from debug-scull
- Replaces linked list with a circular buffer
- includes ioctl reset and status commands

### Usage

```
./load_module.sh
echo asdf > /dev/fifo-scull0
./scull-app /dev/fifo-scull0 -s
cat /dev/fifo-scull0
./scull-app /dev/fifo-scull0 -s
echo asdf > /dev/fifo-scull0
./scull-app /dev/fifo-scull0 -s
./scull-app /dev/fifo-scull0 -r
./scull-app /dev/fifo-scull0 -s
./unload_module.sh
```