UNAME := $(shell uname)
CC = gcc
CFLAGS = -Wall -std=c11 -g
LDFLAGS= -L.

INC = include/
SRC = src/
BIN = bin/
ROOT = ../
PARSER_SRC_FILES = $(wildcard src/SVG*.c)
PARSER_OBJ_FILES = $(patsubst src/SVG%.c,bin/SVG%.o,$(PARSER_SRC_FILES))

XML_PATH = /usr/include/libxml2

parser: $(ROOT)libsvgparser.so

$(ROOT)libsvgparser.so: $(PARSER_OBJ_FILES) $(BIN)LinkedListAPI.o
	gcc -shared -o $(ROOT)libsvgparser.so $(PARSER_OBJ_FILES) $(BIN)LinkedListAPI.o -lxml2 -lm

#Compiles all files named SVG*.c in src/ into object files, places all corresponding SVG*.o files in bin/
$(BIN)SVG%.o: $(SRC)SVG%.c $(INC)LinkedListAPI.h $(INC)SVG*.h
	gcc $(CFLAGS) -I$(XML_PATH) -I$(INC) -c -fpic $< -o $@

$(BIN)liblist.so: $(BIN)LinkedListAPI.o
	$(CC) -shared -o $(BIN)liblist.so $(BIN)LinkedListAPI.o

$(BIN)LinkedListAPI.o: $(SRC)LinkedListAPI.c $(INC)LinkedListAPI.h
	$(CC) $(CFLAGS) -c -fpic -I$(INC) $(SRC)LinkedListAPI.c -o $(BIN)LinkedListAPI.o

clean:
	rm -rf $(BIN)StructListDemo $(BIN)xmlExample $(BIN)*.o
	
###################################################################################################
