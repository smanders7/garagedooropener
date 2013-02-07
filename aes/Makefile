##
## Purpose: Makefile to compile the aes256
## File:    Makefile
## Author:  smanders
## Date:    Feb 6, 2013
##

CC=gcc
LIBS=
CFLAGS=

all: aes256_decrypt

aes256_decrypt: main.o aes256.o
	$(CC) $(CFLAGS) -o aes256_decrypt main.o aes256.o $(LIBS) 

main.o:
	$(CC) $(CFLAGS) -c main.cpp

aes256.o:
	$(CC) $(CFLAGS) -c aes256.cpp

install: aes256_decrypt
	cp aes256_decrypt ../php_code/
	

clean:
	rm -f aes256_decrypt *.o
