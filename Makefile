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
OBJ += $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%arm64.o,$(filter $(SRC_DIR)/%.c,$(SRC)))
OBJ += $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%x86_64.o,$(filter $(SRC_DIR)/%.c,$(SRC)))
LIB := $(LIB_DIR)/libyaml.a $(LIB_DIR)/libbul.a
LIB_SO := $(LIB_DIR)/libyaml.so $(LIB_DIR)/libbul.so

CPPFLAGS:= -I$(INC_DIR)
CFLAGS := -std=gnu89 -O2 -Wall -pedantic -Wextra -Werror
CXXFLAGS:= -std=c++20 -Wall -pedantic -Wextra -Werror -g 
LDFLAGS := -Llib -fsanitize=address
LDLIBS 	:= -lyaml

all: doc $(BIN) $(LIB) 
libs: $(LIB)
cibuildwheel: CFLAGS := -std=gnu99 -O2 -Wall -pedantic -Wextra -fPIC
cibuildwheel: $(LIB)

debug: CPPFLAGS += -DDEBUG -g
debug: doc $(BIN) $(LIB)

$(BIN): $(OBJ) | $(BIN_DIR)
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)/%x86_64.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -target x86_64-apple-macos10.12 -c $< -o $@

$(OBJ_DIR)/%arm64.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -target arm64-apple-macos11 -c $< -o $@

$(BIN_DIR) $(OBJ_DIR) $(LIB_DIR):
	mkdir -p $@

$(LIB_DIR)/libyaml.a: $(GIT_YAML) | $(LIB_DIR)
	cd $(GIT_YAML) && ./bootstrap && CFLAGS="-arch x86_64 -arch arm64" ./configure --with-pic && file src/.libs/libyaml.a
	$(MAKE) -C $(GIT_YAML) 
	cp $(GIT_YAML)/src/.libs/libyaml.a $(LIB_DIR)

$(LIB_DIR)/libyaml.so: $(GIT_YAML) | $(LIB_DIR)
	cd $(GIT_YAML) && ./bootstrap && ./configure --enable-shared
	$(MAKE) -C $(GIT_YAML) 
	if [ -f $(GIT_YAML)/src/.libs/libyaml.dylib ]; then \
		mv $(GIT_YAML)/src/.libs/libyaml.dylib $(GIT_YAML)/src/.libs/libyaml.so; \
	fi
	cp $(GIT_YAML)/src/.libs/libyaml.so $(LIB_DIR)

$(LIB_DIR)/libbul.a: $(OBJ_DIR)/corearm64.o $(OBJ_DIR)/corex86_64.o | $(LIB_DIR)
	lipo -create -output $@ $^

$(LIB_DIR)/libbul.so: $(OBJ_DIR)/core.o $(LIB_DIR)/libyaml.so | $(LIB_DIR)
	$(CC) -shared -o $@ $(OBJ_DIR)/core.o -L$(LIB_DIR) -lyaml

$(GIT_YAML):
	git submodule init $(GIT_YAML)
	git submodule update $(GIT_YAML)

clean:
	@$(RM) -rv $(BIN) $(OBJ) $(OBJ_DIR) $(LIB_DIR)

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


.PHONY: all clean clean_deps doc debug libs cibuildwheel
