CC=g++
CCFLAGS=-g -Wall
LIBS=-lSDL2 -lSDL2main

SRC=src
OBJ=objs

SRCS=$(wildcard $(SRC)/*.cpp)
OBJS=$(patsubst $(SRC)/%.cpp, $(OBJ)/%.o, $(SRCS))

BINDIR=bin
BIN=$(BINDIR)/Wolfboy

$(OBJ)/%.o: $(SRC)/%.cpp $(SRC)/%.h
	$(CC) $(CCFLAGS) -c $< -o $@

$(OBJ)/%.o: $(SRC)/%.cpp
	$(CC) $(CCFLAGS) -c $< -o $@

$(BIN): $(OBJS)
	$(CC) $(CCFLAGS) $(LIBS) $(OBJS) -o $@

all: $(BIN)

clean:
	rm $(OBJ)/* bin/*
