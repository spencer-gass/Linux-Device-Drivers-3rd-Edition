# Scull Module that uses Completions

- Inherits from fifo-scull
- Read empties the buffer or sets a completion if empty
- Write adds to the buffer and clears the completion

### Usage

```
./load_module.sh
echo asdf > /dev/fifo-scull0
cat /dev/fifo-scull0
cat /dev/fifo-scull0
# In a new terminal:
echo asdf > /dev/fifo-scull0
./unload_module.sh
```