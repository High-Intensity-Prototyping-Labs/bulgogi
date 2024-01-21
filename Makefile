SRC_DIR := src
INC_DIR := inc
OBJ_DIR := obj
OBJ_DIR_C := obj_c
BIN_DIR := .

BIN := $(BIN_DIR)/bul
BIN_C := $(BIN_DIR)/bul_c
SRC := $(wildcard $(addsuffix *.cpp, $(SRC_DIR)/))
SRC_C := $(wildcard $(addsuffix *.c, $(SRC_DIR)/))
OBJ := $(addprefix $(OBJ_DIR)/, $(patsubst $(addsuffix %.cpp, $(SRC_DIR)/), %.o, $(SRC)))
OBJ_C := $(addprefix $(OBJ_DIR_C)/, $(patsubst $(addsuffix %.c, $(SRC_DIR)/), %.o, $(SRC_C)))

CPPFLAGS:= -I$(INC_DIR)
CFLAGS 	:= -I$(INC_DIR)
CXXFLAGS:= -std=c++20 -Wall -pedantic -Wextra -Werror -g 
LDFLAGS := -Llib -fsanitize=address
LDLIBS 	:= -lyaml-cpp
LDLIBS_C:= -lyaml-cpp

all: $(BIN) $(BIN_C)
	
$(BIN): $(OBJ) | $(BIN_DIR)
	$(CXX) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(BIN_C): $(OBJ_C) | $(BIN_DIR)
	$(CC) $(LDFLAGS) $^ $(LDLIBS_C) -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR_C)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(BIN_DIR) $(OBJ_DIR) $(OBJ_DIR_C):
	mkdir -p $@

clean:
	@$(RM) -rv $(BIN) $(BIN_C) $(OBJ) $(OBJ_DIR) $(OBJ_DIR_C)

prints:
	echo BIN
	echo '$(BIN)'
	echo SRC
	echo '$(SRC)'
	echo OBJ
	echo '$(OBJ)'

.PHONY: all clean
