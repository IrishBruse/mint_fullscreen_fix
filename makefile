# Makefile for mint_fullscreen_fix

CC = gcc
CFLAGS = -Wall -O2
LIBS = -lX11 -lXrandr

TARGET = mint_fullscreen_fix
SRC = mint_fullscreen_fix.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC) $(LIBS)

clean:
	rm -f $(TARGET)
