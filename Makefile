# Compiler
CC   := gcc

# *.c
SRC  := src
# *.o
OBJ  := obj
# *.a/*.dll/*.so
LIB  := lib
# Binaries
BIN  := bin

# Those are user-defined names
# Shared object file
PACK := libptahka32
# Main source and generated executable
MAIN := main

# The wildcard function automatically detects all sources files in a given directory
SOURCES := $(wildcard $(SRC)/*.c)
# The following rule simply replaces the extension of all source files to get the object files
# OBJECTS := $(SOURCES:.c=.o)
# The patsubst function replaces a data for another based on a pattern
OBJECTS := $(patsubst $(SRC)/%.c,$(OBJ)/%.o, $(SOURCES))
# The -I . flag automatically detects any header dependency files in a given directory
CFLAGS  := -I$(SRC)
DFLAGS  := -lwsock32 -lws2_32

$(OBJ)/%.o: $(SRC)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: all

all: $(OBJECTS)
	ar rcs $(PACK).a $^
	$(CC) -c $(MAIN).c -o $(OBJ)/$(MAIN).o $(DFLAGS)
	$(CC) -shared -o $(PACK).dll -Wl,--out-implib,$(PACK).a $^ $(DFLAGS)
	$(CC) -o $(MAIN) $(MAIN).c -Wl,-rpath=. -L. -l$(PACK)
# make -B all