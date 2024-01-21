SRC_DIR := src
INC_DIR := inc
OBJ_DIR := obj
BIN_DIR := .

BIN := $(BIN_DIR)/bul
SRC := $(wildcard $(addsuffix *.cpp, $(SRC_DIR)/))
OBJ := $(addprefix $(OBJ_DIR)/, $(patsubst $(addsuffix %.cpp, $(SRC_DIR)/), %.o, $(SRC)))

CPPFLAGS:= -I$(INC_DIR)
CXXFLAGS:= -std=c++20 -Wall -pedantic -Wextra -Werror -g 
LDFLAGS := -Llib -fsanitize=address
LDLIBS 	:= -lyaml-cpp

all: $(BIN)
	
$(BIN): $(OBJ) | $(BIN_DIR)
	$(CXX) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

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

.PHONY: all clean
