# Basic Scull Module

- Simple Character Utility for Loading Localities (SCULL)
- Initial implementation
    - Linked list data structure in kernel memory serves as the device
    - initialized and registers a character device
    - open, read, write, release file operations

### Usage

```
./load_module.sh
echo asdf > /dev/basic-scull0
cat /dev/basic-scull0
/unload_module.sh
```