SRC_DIR := src
INC_DIR := inc
OBJ_DIR := obj
LIB_DIR := lib
BIN_DIR := .

GIT_YAML := libyaml

BIN := $(BIN_DIR)/bul
SRC := $(wildcard $(addsuffix *.cpp, $(SRC_DIR)/))
SRC += $(wildcard $(addsuffix *.c, $(SRC_DIR)/))
OBJ := $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(filter $(SRC_DIR)/%.cpp,$(SRC)))
OBJ += $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(filter $(SRC_DIR)/%.c,$(SRC)))
LIB := $(LIB_DIR)/libyaml.a

CPPFLAGS:= -I$(INC_DIR)
CFLAGS := -Wall -pedantic -Wextra -Werror -g
CXXFLAGS:= -std=c++20 -Wall -pedantic -Wextra -Werror -g 
LDFLAGS := -Llib -fsanitize=address
LDLIBS 	:= -lyaml

all: doc $(BIN) $(LIB) 

debug: CPPFLAGS += -DDEBUG
debug: doc $(BIN) $(LIB)

$(BIN): $(OBJ) | $(BIN_DIR)
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(BIN_DIR) $(OBJ_DIR) $(LIB_DIR):
	mkdir -p $@

$(LIB_DIR)/libyaml.a: $(GIT_YAML) | $(LIB_DIR)
	cd $(GIT_YAML) && ./bootstrap && ./configure --with-pic
	$(MAKE) -C $(GIT_YAML) 
	cp $(GIT_YAML)/src/.libs/libyaml.a $(LIB_DIR)

$(GIT_YAML):
	git submodule init $(GIT_YAML)
	git submodule update $(GIT_YAML)

clean:
	@$(RM) -rv $(BIN) $(OBJ) $(OBJ_DIR)

clean_deps:
	@$(RM) -rv $(LIB) $(GIT_YAML)

prints:
	echo BIN
	echo '$(BIN)'
	echo SRC
	echo '$(SRC)'
	echo OBJ
	echo '$(OBJ)'

doc:
	doxygen doxygen > /dev/null 2> /dev/null


.PHONY: all clean clean_deps doc debug
