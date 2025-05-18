# Simple Hardware Operations and Raw Tests (Short)

- Load time parameter to use one of three modes:
    - 0: Use IO port
    - 1: Remap IO port to MMIO
    - 2: Use memory mapped IO
- I'm only compiling and not running this one since it requires HW.

### Usage

```
./load_module.sh
ll /dev
./unload_module.sh
```