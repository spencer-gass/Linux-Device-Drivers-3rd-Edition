.PHONY: all clean driver install
BIN_DIR = bin

all: driver install

driver:
	$(MAKE) -C driver

install: driver
	@mkdir -p $(BIN_DIR)
	@cp driver/*.ko $(BIN_DIR)

clean:
	$(MAKE) -C driver clean
	rm -f $(BIN_DIR)/*.ko
