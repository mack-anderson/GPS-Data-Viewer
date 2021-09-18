UNAME := $(shell uname)
CC = gcc
CFLAGS = -Wall -std=c11 -g
LDFLAGS= -L.

INC = include/
SRC = src/
PRO = ../
PARSER_SRC_FILES = $(wildcard src/GPX*.c)
PARSER_OBJ_FILES = $(patsubst src/GPX%.c,GPX%.o,$(PARSER_SRC_FILES))

ifeq ($(UNAME), Linux)
	XML_PATH = /usr/include/libxml2
endif
ifeq ($(UNAME), Darwin)
	XML_PATH = /System/Volumes/Data/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk/usr/include/libxml2
endif

parser: $(PRO)libgpxparser.so

$(PRO)libgpxparser.so: $(PARSER_OBJ_FILES) LinkedListAPI.o
	gcc -shared -o $(PRO)libgpxparser.so $(PARSER_OBJ_FILES) LinkedListAPI.o -lxml2 -lm

#Compiles all files named GPX*.c in src/ into object files, places all coresponding GPX*.o files in bin/
GPX%.o: $(SRC)GPX%.c $(INC)LinkedListAPI.h $(INC)GPX*.h
	gcc $(CFLAGS) -I$(XML_PATH) -I$(INC) -c -fpic $< -o $@

liblist.so: LinkedListAPI.o
	$(CC) -shared -o liblist.so LinkedListAPI.o

LinkedListAPI.o: $(SRC)LinkedListAPI.c $(INC)LinkedListAPI.h
	$(CC) $(CFLAGS) -c -fpic -I$(INC) $(SRC)LinkedListAPI.c -o LinkedListAPI.o

clean:
	rm -rf StructListDemo xmlExample *.o *.so $(PRO)*.so

#This is the target for the in-class XML example
xmlExample: $(SRC)libXmlExample.c
	$(CC) $(CFLAGS) -I$(XML_PATH) $(SRC)libXmlExample.c -lxml2 -o xmlExample

#These are sample targets for the list demo code incldued in the class examples.  They will not be used
#for A1, but they can help you figure out who to set up a target for your own test main

StructListDemo: StructListDemo.o liblist.so
	$(CC) $(CFLAGS) $(LDFLAGS) -L -o StructListDemo StructListDemo.o  -llist
	
StructListDemo.o: $(SRC)StructListDemo.c
	$(CC) $(CFLAGS) -I$(INC) -c $(SRC)StructListDemo.c -o StructListDemo.o

###################################################################################################

mainTest: mainTest.o liblist.so libgpxparser.so
	$(CC) $(CFLAGS) -I$(XML_PATH) $(LDFLAGS) -L -lxml2 -o mainTest mainTest.o  -llist -lgpxparser
	
mainTest.o: $(SRC)mainTest.c
	$(CC) $(CFLAGS) -I$(XML_PATH) -I$(INC) -c $(SRC)mainTest.c -lxml2 -o mainTest.o