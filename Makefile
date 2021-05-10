CC=g++
CCFLAGS=-Wall
LIBS=-lSDL2 -lSDL2main

SRC=src
OBJ=objs

SRCS=$(wildcard $(SRC)/*.cpp)
OBJS=$(patsubst $(SRC)/%.cpp, $(OBJ)/%.o, $(SRCS))

BINDIR=bin
BIN=$(BINDIR)/Wolfboy

$(OBJ)/%.o: $(SRC)/%.cpp $(SRC)/%.h directories
	$(CC) $(CCFLAGS) -c $< -o $@

$(OBJ)/%.o: $(SRC)/%.cpp directories
	$(CC) $(CCFLAGS) -c $< -o $@

$(BIN): directories $(OBJS)
	$(CC) $(CCFLAGS) $(LIBS) $(OBJS) -o $@

directories:
	mkdir -p $(BINDIR) $(OBJ)

all: $(BIN)

clean:
	rm $(OBJ)/* bin/*
