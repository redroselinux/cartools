CXX := g++
CXXFLAGS := -Wall -Wextra -Werror -std=c++20 -O3
STRIP := strip
SRC_DIR := src
BIN_DIR := bin
SRCS := $(wildcard $(SRC_DIR)/*.cpp)
BINS := $(patsubst $(SRC_DIR)/%.cpp,$(BIN_DIR)/%,$(SRCS))

.PHONY: all
all: $(BIN_DIR) $(BINS)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

$(BIN_DIR)/%: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) $< -o $@

DESTDIR := /
PREFIX := /usr/
BINDIR := /bin/

.PHONY: install
install: $(BIN_DIR)
	mkdir -p $(DESTDIR)/$(PREFIX)/$(BINDIR)
	install -s -v -m=0755 --strip-program=$(STRIP) $(BIN_DIR)/* $(DESTDIR)/$(PREFIX)/$(BINDIR)

.PHONY: clean
clean:
	rm -rf $(BIN_DIR)
