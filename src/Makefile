SRC_DIR := .
OBJ_DIR := ../obj
BIN_DIR := ..

BIN := $(BIN_DIR)/bul
SRC := $(wildcard *.cpp)
OBJ := $(addprefix $(OBJ_DIR)/, $(patsubst %.cpp, %.o, $(SRC)))

CPPFLAGS=-I../inc
CXXFLAGS=-std=c++20 -Wall -pedantic

all: $(BIN)
	
$(BIN): $(OBJ) | $(BIN_DIR)
	$(CXX) $^ -o $@

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
