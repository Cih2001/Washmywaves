BIN_NAME := washmywaves

GPP := g++

SRC_DIR := ./src
OBJ_DIR := ./obj
CXX_FILES := $(shell find $(SRC_DIR) | grep ".cc$$" | xargs echo)
OBJ_FILES := $(patsubst $(SRC_DIR)/%.cc,$(OBJ_DIR)/%.o,$(CXX_FILES))
OBJ_DIRS := $(subst /,/,$(sort $(dir $(OBJ_FILES))))

LD_FLAGS := ./lib/libmp3lame.a -lpthread
CXXFLAGS := -Wall -c -I$(SRC_DIR) -s -O3 -std=c++17

all: CXXFLAGS += -g -s
all: LD_FLAGS += -s
all: clean makedir link

debug: CXXFLAGS += -DDEBUG -g
debug: LD_FLAGS += -DDEBUG -g
debug: BIN_NAME := $(BIN_NAME)_dbg
debug: clean makedir link

link: $(OBJ_FILES)
	$(GPP) -o $(BIN_NAME) $^ $(LD_FLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cc
	$(GPP) -o $@ $< $(CXXFLAGS) 

.PHONY: clean
clean: 
	rm -rf $(OBJ_DIR)
	rm -f $(BIN_NAME)*

makedir:
	mkdir -p $(OBJ_DIRS)
