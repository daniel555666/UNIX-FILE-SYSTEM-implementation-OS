.PHONY: all run clean
CC = gcc
FLAGS= 
HEADERS = 
all: main

main: main.o lib.so
	$(CC) main.o lib.so -o main

lib.so: file_system.o mylibc.o 
	$(CC) --shared -fPIC -g file_system.o mylibc.o  -o lib.so

%.o: %.c 
	$(CC) -c $< -o $@

clean:
	rm -f *.o main