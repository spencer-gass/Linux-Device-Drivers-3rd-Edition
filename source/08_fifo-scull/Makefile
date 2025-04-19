.PHONY: all clean driver user_app install
BIN_DIR = bin

all: driver user_app install

driver:
	$(MAKE) -C driver

user_app:
	$(MAKE) -C user_app BIN_NAME="scull-app"

install: driver user_app
	@mkdir -p $(BIN_DIR)
	@cp driver/*.ko $(BIN_DIR)
	@cp user_app/scull-app $(BIN_DIR)

clean:
	$(MAKE) -C driver clean
	$(MAKE) -C user_app clean
	rm -f $(BIN_DIR)/*.ko
	rm -f $(BIN_DIR)/scull-app
