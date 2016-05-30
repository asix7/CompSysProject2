# Andres Landeta alandeta 631427 
# Makefile for project 2 (COMP30017)
#

## CC  = Compiler.
## CFLAGS_C = Compiler flags for client.
## CFLAGS_S = Compiler flags for server.
CC  = gcc
CFLAGS  = -g 
CFLAGS_S  = -g -lnsl -pthread

## OBJ_C = Object files for client.
## EXE_C = Executable name for client.
## OBJ_S = Object files for server.
## EXE_S = Executable name for server.

OBJ_C =		client.o 
EXE_C =		client
OBJ_S =		server.o game.o 
EXE_S =		server

all: client server

$(EXE_C): $(OBJ_C)
	$(CC) $(CFLAGS) -o client $(OBJ_C) -lm

$(EXE_S): $(OBJ_S) 
	$(CC) $(CFLAGS_S) -o server $(OBJ_S) -lm

## Clean: Remove object files and core dump files.
clean:
		-/bin/rm $(OBJ_C) 
		-/bin/rm $(OBJ_S) 

## Clobber: Performs Clean and removes executable files.
clobber: clean
		-/bin/rm $(EXE_C)
		-/bin/rm $(EXE_S)

## Dependencies
server.o: server.h game.h 
game.o: game.h	
