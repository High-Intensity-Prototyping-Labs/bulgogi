CC=clang++
BIN=bul
SRC=$(wildcard *.cc)
FLAGS=-std=c++20 -Wall

all:
	$(CC) -o $(BIN) $(FLAGS) $(SRC)

analyze:
	$(CC) --analyze -Xclang -analyzer-output=html -o analyzer -std=c++20 $(SRC)
	open -a "Google Chrome Beta" analyzer/index.html

clean:
	rm $(BIN)

.phony: all
