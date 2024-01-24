SRC_DIR := src
INC_DIR := inc
OBJ_DIR := obj
BIN_DIR := .

BIN := $(BIN_DIR)/bul
SRC := $(wildcard $(addsuffix *.cpp, $(SRC_DIR)/))
SRC += $(wildcard $(addsuffix *.c, $(SRC_DIR)/))
OBJ := $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(filter $(SRC_DIR)/%.cpp,$(SRC)))
OBJ += $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(filter $(SRC_DIR)/%.c,$(SRC)))

CPPFLAGS:= -I$(INC_DIR)
CFLAGS := -Wall -pedantic -Wextra -Werror -g
CXXFLAGS:= -std=c++20 -Wall -pedantic -Wextra -Werror -g 
LDFLAGS := -Llib -fsanitize=address
LDLIBS 	:= -lyaml

all: $(BIN)
	
$(BIN): $(OBJ) | $(BIN_DIR)
	$(CXX) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(BIN_DIR) $(OBJ_DIR):
	mkdir -p $@

clean:
	@$(RM) -rv $(BIN) $(OBJ) $(OBJ_DIR)

prints:
	echo BIN
	echo '$(BIN)'
	echo SRC
	echo '$(SRC)'
	echo OBJ
	echo '$(OBJ)'

doc:
	doxygen doxygen

.PHONY: all clean doc
