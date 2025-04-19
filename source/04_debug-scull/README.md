# Scull Module with Debug

- Inherits from basic-scull
- Adds compile time option to include debug prints
- Adds scripts to change terminal log level to show or hide debug prints

### Usage

```
./load_module.sh
./enable_debug.sh
echo asdf > /dev/debug-scull0
cat /dev/debug-scull0
./disable_debug.sh
./unload_module.sh
```