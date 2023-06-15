SRC_DIR = src
OBJ_DIR = obj
PROG_PATH = sleep_server

INCLUDE_FILES = $(wildcard $(SRC_DIR)/*.h)
OBJ_FILES = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(wildcard $(SRC_DIR)/*.c))

LINK_FLAGS =
C_FLAGS = -g -pthread

$(PROG_PATH): $(OBJ_FILES)
	$(CC) -o $@ $^ $(C_FLAGS) $(LINK_FLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c $(INCLUDE_FILES)
	mkdir -p $(OBJ_DIR)
	$(CC) -o $@ -c $< $(C_FLAGS)

clean:
	rm -rf $(PROG_PATH) $(OBJ_DIR)
