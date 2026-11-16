CXX ?= c++
CXXFLAGS ?= -std=c++20 -O2 -Wall -Wextra -Wpedantic
CPPFLAGS += -Iinclude
ifeq ($(shell uname -s),Darwin)
SDKROOT ?= $(shell test -d /Library/Developer/CommandLineTools/SDKs/MacOSX26.5.sdk && echo /Library/Developer/CommandLineTools/SDKs/MacOSX26.5.sdk)
export SDKROOT
endif
BUILD := build
APP := $(BUILD)/lr1
TESTS := $(BUILD)/tree_tests
HEADERS := $(wildcard include/*.hpp)

.PHONY: all run test test-sanitize benchmark graph clean
all: $(APP)
$(BUILD):
	mkdir -p $@
$(APP): src/main.cpp src/tree_console.cpp src/benchmark.cpp $(HEADERS) | $(BUILD)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) src/main.cpp src/tree_console.cpp src/benchmark.cpp -o $@
$(TESTS): tests/test_tree.cpp $(HEADERS) | $(BUILD)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $< -o $@
run: $(APP)
	./$(APP)
test: $(TESTS)
	./$(TESTS)
test-sanitize: | $(BUILD)
	$(CXX) $(CPPFLAGS) -std=c++20 -O1 -g -Wall -Wextra -Wpedantic -fsanitize=address,undefined -fno-omit-frame-pointer tests/test_tree.cpp -o $(BUILD)/tree_tests_san
	./$(BUILD)/tree_tests_san
benchmark: $(APP)
	./$(APP) --benchmark $(or $(MAX_SIZE),10000000) $(or $(SEED),20261116)
graph:
	python3 plot_benchmark.py
clean:
	rm -rf $(BUILD) benchmark.csv benchmark.svg
