CXX      = g++
CXXFLAGS = -std=gnu++20 -Wall -Wextra -O2 -g
LDFLAGS  = -pthread

SRC_DIR  = src
BINARIES = mn fn1 fn2

all: $(BINARIES)

mn: $(SRC_DIR)/mn.cpp $(SRC_DIR)/common.hpp $(SRC_DIR)/ipc.hpp $(SRC_DIR)/task.hpp
	$(CXX) $(CXXFLAGS) -o $@ $(SRC_DIR)/mn.cpp $(LDFLAGS)

fn1: $(SRC_DIR)/fn1.cpp $(SRC_DIR)/common.hpp $(SRC_DIR)/ipc.hpp
	$(CXX) $(CXXFLAGS) -o $@ $(SRC_DIR)/fn1.cpp $(LDFLAGS)

fn2: $(SRC_DIR)/fn2.cpp $(SRC_DIR)/common.hpp $(SRC_DIR)/ipc.hpp
	$(CXX) $(CXXFLAGS) -o $@ $(SRC_DIR)/fn2.cpp $(LDFLAGS)

clean:
	rm -f $(BINARIES)

.PHONY: all clean