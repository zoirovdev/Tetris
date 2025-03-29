#!/bin/bash


# Compiler 
CC=gcc


# Source file
SRC=main.c


# Output binary
OUT=game


# Compiler flags
CFLAGS="-Wall -Wextra -std=c11"


# Raylib flags 
LDFLAGS="-lraylib -lm -ldl -lpthread -lGL -lX11"


# Build the project
$CC $CFLAGS $SRC -o $OUT $LDFLAGS


# Check if compilation succeeded
if [ $? -eq 0 ]; then
    echo "Build successful! Run ./$OUT"
else 
    echo "Build failed!"
fi
