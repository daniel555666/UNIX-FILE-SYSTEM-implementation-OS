.PHONY: all run clean
CC = gcc -g
FLAGS= 
HEADERS = 

all: libmylibc.so libmyfs.so test  

test: test.o libmylibc.so libmyfs.so
	$(CC) test.o ./libmylibc.so ./libmyfs.so -o test -fPIC

libmyfs.so: file_system.o
	$(CC) file_system.o -shared -o libmyfs.so -fPIC

libmylibc.so: mylibc.o
	$(CC) mylibc.o -shared -o libmylibc.so -fPIC

mylibc.o:
	$(CC) -c mylibc.c -fPIC

%.o: %.c 
	$(CC) -c $< -o $@ -fPIC

clean:
	rm -f *.o test