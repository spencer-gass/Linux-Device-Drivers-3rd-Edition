# Scull Module with ioctl Commands

- Inherits from debug-scull
- Adds reset, status, and write commands via ioctl
- Adds user app for making ioctl command requests

### Usage

```
./load_module.sh
echo asdf > /dev/ioctl-scull0
./scull-app /dev/ioctl-scull0 -s
./scull-app /dev/ioctl-scull0 -s
./scull-app /dev/ioctl-scull0 -w fdsa
cat /dev/ioctl-scull0
./unload_module.sh
```