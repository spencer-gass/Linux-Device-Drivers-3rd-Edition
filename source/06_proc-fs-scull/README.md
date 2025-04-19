# Scull Module with procfs Entry

- Inherits from debug-scull
- Adds a procfs file entry to read status from

### Usage

```
./load_module.sh
echo asdf > /dev/proc-fs-scull0
cat /dev/proc-fs-scull0
cat /proc/scull/scull0
./unload_module.sh
```