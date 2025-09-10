BUILD_DIR := build

.PHONY: all configure build test update clean

all: build

configure:
	cmake -S . -B $(BUILD_DIR)

build: configure
	cmake --build $(BUILD_DIR) --parallel

test: build
	ctest --test-dir $(BUILD_DIR) --output-on-failure

update: build
	$(BUILD_DIR)/test_runner --update

clean:
	rm -rf $(BUILD_DIR) test/out


