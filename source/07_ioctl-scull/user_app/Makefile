BIN_NAME ?= scull-app

all:
	gcc -I../driver -static -o $(BIN_NAME) main.c

.PHONY: clean

clean:
	rm -f $(BIN_NAME)