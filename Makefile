# Makefile

CC = g++
CFLAGS = -std=c++17 -Wall -Iinclude -pthread -g

SRC_DIR = src
OBJ_DIR = obj
INCLUDE_DIR = include

SERVER_SRC = $(wildcard $(SRC_DIR)/*.cpp)
SERVER_OBJ = $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SERVER_SRC))

MAIN_SRC = main.cpp
MAIN_OBJ = $(OBJ_DIR)/main.o

EXECUTABLE = SleepManager

.PHONY: all clean

all: $(EXECUTABLE)

$(EXECUTABLE): $(SERVER_OBJ) $(MAIN_OBJ)
	$(CC) $(CFLAGS) -o $@ $^

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CC) $(CFLAGS) -I$(INCLUDE_DIR) -c -o $@ $<

$(OBJ_DIR)/main.o: $(MAIN_SRC) | $(OBJ_DIR)
	$(CC) $(CFLAGS) -I$(INCLUDE_DIR) -c -o $@ $<

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

clean:
	rm -rf $(OBJ_DIR) $(EXECUTABLE)
