BIN_NAME ?= scull-app
OBJ = main.o test.o
CFLAGS = -static -I../driver

all: $(BIN_NAME)

$(BIN_NAME): $(OBJ)
	gcc $(CFLAGS) -o $(BIN_NAME) $(OBJ)

main.o: main.c
	gcc $(CFLAGS) -c main.c

test.o: test.c test.h
	gcc -pthread -c test.c

clean:
	rm -f $(BIN_NAME) $(OBJ)

.PHONY: clean all
