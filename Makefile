SRC_DIR := src 
INC_DIR := inc
BIN_DIR := .

BIN := $(BIN_DIR)/bul
SRC := $(wildcard $(SRC_DIR)/*.cpp)
OBJ := $(SRC:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)

CPPFLAGS=-Iinc
CXXFLAGS=-std=c++20 -Wall

all: $(BIN)
	
$(BIN): $(OBJ) | $(BIN_DIR)
	echo $^
	echo $@
	echo $(OBJ)
	echo $(SRC)
	$(CXX) $^ -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

$(BIN_DIR) $(OBJ_DIR):
	mkdir -p $@

clean:
	@$(RM) -rv $(BIN_DIR) $(OBJ_DIR) 

.PHONY: all clean
