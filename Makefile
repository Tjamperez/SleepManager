SRC_DIR = src
OBJ_DIR = obj
PROG_PATH = sleep_server

INCLUDE_FILES = $(wildcard $(SRC_DIR)/*.h)
OBJ_FILES = $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(wildcard $(SRC_DIR)/*.cpp))

LINK_FLAGS =
CPP = g++
C_FLAGS = -g -pthread

$(PROG_PATH): $(OBJ_FILES)
	$(CPP) -o $@ $^ $(C_FLAGS) $(LINK_FLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp $(INCLUDE_FILES)
	mkdir -p $(OBJ_DIR)
	$(CPP) -o $@ -c $< $(C_FLAGS)

clean:
	rm -rf $(PROG_PATH) $(OBJ_DIR)