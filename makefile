.PHONY: all run clean
CC = gcc
FLAGS= 
HEADERS = 
all: main

main: main.o file_system.o
	$(CC) main.o file_system.o -o main

%.o: %.c 
	$(CC) -c $< -o $@

clean:
	rm -f *.o main